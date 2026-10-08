// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * LX2160A 28G-Lynx SerDes inspection (`lx2160_serdes`).
 */

#include <command.h>
#include <hwconfig.h>
#include <vsprintf.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_lsch3.h>
#include <asm/arch/soc.h>		/* gur_in32() picks the GUR endianness */
#include <fsl-mc/fsl_mc.h>
#include <linux/bitmap.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>

#include "serdes_split.h"

#if defined(CONFIG_SYS_NXP_SRDS_3)
#define SRDS_COUNT		3
#else
#define SRDS_COUNT		2	/* LX2162A has no SerDes 3 */
#endif
#define SRDS_LANES		8
#define SRDS_BLOCK_STRIDE	0x10000
#define SRDS_LANE_STRIDE	0x100
#define SRDS_PLL_STRIDE		0x100

#define SRDS_PLLnRSTCTL(b, n)	((b) + 0x400 + (n) * SRDS_PLL_STRIDE)
#define SRDS_PLLnCR0(b, n)	((b) + 0x404 + (n) * SRDS_PLL_STRIDE)
#define SRDS_PLLnCR1(b, n)	((b) + 0x408 + (n) * SRDS_PLL_STRIDE)
#define SRDS_PLLnCR5(b, n)	((b) + 0x414 + (n) * SRDS_PLL_STRIDE)
#define SRDS_LNmGCR0(b, m)	((b) + 0x800 + (m) * SRDS_LANE_STRIDE)
#define SRDS_LNmTRSTCTL(b, m)	((b) + 0x820 + (m) * SRDS_LANE_STRIDE)
#define SRDS_LNmTGCR0(b, m)	((b) + 0x824 + (m) * SRDS_LANE_STRIDE)
#define SRDS_LNmRRSTCTL(b, m)	((b) + 0x840 + (m) * SRDS_LANE_STRIDE)
#define SRDS_LNmRGCR0(b, m)	((b) + 0x844 + (m) * SRDS_LANE_STRIDE)
#define SRDS_PCCC(b)		((b) + 0x10B0)
#define SRDS_PCCD(b)		((b) + 0x10B4)
#define SRDS_PCCE(b)		((b) + 0x10B8)

/* LNmGCR0 */
#define GCR0_PORT_RST_LEFT	BIT(17)
#define GCR0_PORT_LN0_B		BIT(16)
#define GCR0_PROTO_SEL_MASK	0x000000F8
#define GCR0_PROTO_SEL_SHIFT	3
#define GCR0_IF_WIDTH_MASK	0x00000007

/* LNmTRSTCTL and LNmRRSTCTL share a layout */
#define RSTCTL_RST_REQ		BIT(31)
#define RSTCTL_RST_DONE		BIT(30)
#define RSTCTL_HLT_REQ		BIT(27)
#define RSTCTL_STP_REQ		BIT(26)
#define RSTCTL_DIS		BIT(24)

/* PLLnRSTCTL adds two status bits the lane RSTCTLs do not have */
#define PLLRSTCTL_RST_ERR	BIT(29)		/* no lock before time-out  */
#define PLLRSTCTL_LOCK		BIT(23)		/* PLL calibrated + locked  */

/* PCCE: E40Ga at 30-28 (LRV 31), E40Gb at 26-24 (LRV 27) */
#define PCCE_E40GA_CFG(v)	(((v) >> 28) & 0x7)
#define PCCE_E40GB_CFG(v)	(((v) >> 24) & 0x7)

/* PCCE: E100Ga at 14-12 (LRV 15), E100Gb at 10-8 (LRV 11) */
#define PCCE_E100GA_CFG(v)	(((v) >> 12) & 0x7)
#define PCCE_E100GA_LRV(v)	(((v) >> 15) & 0x1)
#define PCCE_E100GB_CFG(v)	(((v) >>  8) & 0x7)
#define PCCE_E100GB_LRV(v)	(((v) >> 11) & 0x1)

/* PCCD: E25Ga..E25Gh, 3 bits each, a at 30-28 down to h at 2-0 */
#define PCCD_E25G_CFG(v, i)	(((v) >> (28 - 4 * (i))) & 0x7)

static unsigned long srds_base(int sd)
{
	return CFG_SYS_FSL_LSCH3_SERDES_ADDR + (unsigned long)sd * SRDS_BLOCK_STRIDE;
}

static const char *proto_sel_name(u32 sel)
{
	switch (sel) {
	case 0x00: return "PCIe";
	case 0x01: return "SGMII/1000Base-KX";
	case 0x02: return "SATA";
	case 0x0A: return "XFI/SFI/10GBase-R/KR, 10G-SXGMII, 40GBase-R/KR";
	case 0x1A: return "25GBase-R/KR, 50GAUI2, 100G CAUI4";
	default:   return "reserved";
	}
}

static const char *if_width_name(u32 w)
{
	switch (w) {
	case 0: return "10-bit";
	case 1: return "16-bit";
	case 2: return "20-bit";
	case 3: return "32-bit";
	case 4: return "40-bit";
	default: return "?";
	}
}

static void print_rstctl(const char *what, u32 v)
{
	printf("      %s %08x  [%s%s%s%s%s ]\n", what, v,
	       (v & RSTCTL_RST_REQ)  ? " RST_REQ"  : "",
	       (v & RSTCTL_RST_DONE) ? " RST_DONE" : "",
	       (v & RSTCTL_HLT_REQ)  ? " HLT_REQ"  : "",
	       (v & RSTCTL_STP_REQ)  ? " STP_REQ"  : "",
	       (v & RSTCTL_DIS)      ? " DIS"      : "");
}

/*
 * PORT_LN0_B is the single most informative bit here. It
 * must be 0 only on the port's master lane, and 0 on every single-lane
 * port. So a run of lanes where exactly one has PORT_LN0_B=0 is one
 * multi-lane port, and a run where all of them are 0 is that many
 * one-lane ports. That is precisely what the 100G -> 4x 25G split
 * changes, and it is invisible in RCWSR.
 */
static void print_lane(unsigned long base, int m)
{
	u32 gcr0 = in_le32((void __iomem *)SRDS_LNmGCR0(base, m));
	u32 sel  = (gcr0 & GCR0_PROTO_SEL_MASK) >> GCR0_PROTO_SEL_SHIFT;
	u32 wid  = gcr0 & GCR0_IF_WIDTH_MASK;

	printf("  lane %d (LN%c)\n", m, 'A' + m);
	printf("      GCR0    %08x  PORT_RST_LEFT=%d PORT_LN0_B=%d (%s)\n",
	       gcr0,
	       (gcr0 & GCR0_PORT_RST_LEFT) ? 1 : 0,
	       (gcr0 & GCR0_PORT_LN0_B) ? 1 : 0,
	       (gcr0 & GCR0_PORT_LN0_B) ? "follows a master lane"
					: "port master / single-lane port");
	printf("                        PROTO_SEL=%02x (%s) IF_WIDTH=%d (%s)\n",
	       sel, proto_sel_name(sel), wid, if_width_name(wid));
	print_rstctl("TRSTCTL", in_le32((void __iomem *)SRDS_LNmTRSTCTL(base, m)));
	print_rstctl("RRSTCTL", in_le32((void __iomem *)SRDS_LNmRRSTCTL(base, m)));
}

/*
 * Summarise the port grouping, which is the one line that says whether a
 * split happened. With PORT_RST_LEFT=0 the port's master is
 * its right-most lane (towards LNH), with PORT_RST_LEFT=1 it is the
 * left-most: PORT_LN0_B=0 marks that master, and is also 0 on every
 * single-lane port. So a group is a run of PORT_LN0_B=1 lanes terminated
 * (or led) by a PORT_LN0_B=0 lane.
 *
 * Checked on board with the PBI split applied:
 *   [LNA-LND] [LNE] [LNF] [LNG] [LNH]   -- 1x 100G + 4x 25G
 * and without it lanes E-H would read as a second [LNE-LNH] group.
 */
static void print_grouping(unsigned long base)
{
	u32 gcr0[SRDS_LANES];
	bool claimed[SRDS_LANES];
	int m, orphan = 0;

	for (m = 0; m < SRDS_LANES; m++) {
		gcr0[m] = in_le32((void __iomem *)SRDS_LNmGCR0(base, m));
		claimed[m] = false;
	}

	printf("  port grouping (PORT_LN0_B)  ");
	for (m = 0; m < SRDS_LANES; m++) {
		int s, e;

		if (gcr0[m] & GCR0_PORT_LN0_B)
			continue;               /* a follower, not a master */

		s = m;
		e = m;
		if (gcr0[m] & GCR0_PORT_RST_LEFT) {
			while (e + 1 < SRDS_LANES &&
			       (gcr0[e + 1] & GCR0_PORT_LN0_B))
				e++;
		} else {
			while (s - 1 >= 0 && (gcr0[s - 1] & GCR0_PORT_LN0_B))
				s--;
		}

		if (s == e)
			printf("[LN%c] ", 'A' + s);
		else
			printf("[LN%c-LN%c] ", 'A' + s, 'A' + e);

		while (s <= e)
			claimed[s++] = true;
	}

	for (m = 0; m < SRDS_LANES; m++)
		if (!claimed[m])
			orphan++;
	if (orphan)
		printf(" (%d lane%s claimed by no master)", orphan,
		       orphan == 1 ? "" : "s");
	printf("\n");
}

static void print_converters(unsigned long base)
{
	u32 pccc = in_le32((void __iomem *)SRDS_PCCC(base));
	u32 pccd = in_le32((void __iomem *)SRDS_PCCD(base));
	u32 pcce = in_le32((void __iomem *)SRDS_PCCE(base));
	int i, any = 0;

	printf("  protocol converters\n");
	printf("      PCCC    %08x  (SXGMII/XFI)\n", pccc);
	printf("      PCCD    %08x  (E25G)  ", pccd);
	for (i = 0; i < SRDS_LANES; i++) {
		u32 cfg = PCCD_E25G_CFG(pccd, i);

		if (cfg)
			printf("%sE25G%c=%d", any++ ? " " : "", 'a' + i, cfg);
	}
	printf("%s\n", any ? "" : "none enabled");

	printf("      PCCE    %08x  (E40G/E50G/E100G)  E100Ga=%d%s E100Gb=%d%s\n",
	       pcce,
	       PCCE_E100GA_CFG(pcce), PCCE_E100GA_LRV(pcce) ? " (lane-reversed)" : "",
	       PCCE_E100GB_CFG(pcce), PCCE_E100GB_LRV(pcce) ? " (lane-reversed)" : "");
}

static void print_rcw_view(int sd)
{
	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 cfg;
	int regsr, shift;
	u32 mask;

	switch (sd) {
	case FSL_SRDS_1:
		regsr = FSL_CHASSIS3_SRDS1_REGSR;
		mask  = FSL_CHASSIS3_SRDS1_PRTCL_MASK;
		shift = FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;
		break;
	case FSL_SRDS_2:
		regsr = FSL_CHASSIS3_SRDS2_REGSR;
		mask  = FSL_CHASSIS3_SRDS2_PRTCL_MASK;
		shift = FSL_CHASSIS3_SRDS2_PRTCL_SHIFT;
		break;
#if defined(CONFIG_SYS_NXP_SRDS_3)
	case NXP_SRDS_3:
		regsr = FSL_CHASSIS3_SRDS3_REGSR;
		mask  = FSL_CHASSIS3_SRDS3_PRTCL_MASK;
		shift = FSL_CHASSIS3_SRDS3_PRTCL_SHIFT;
		break;
#endif
	default:
		return;
	}

	cfg = (gur_in32(&gur->rcwsr[regsr - 1]) & mask) >> shift;
	printf("  RCWSR%-2d  raw=0x%04x -> protocol %d\n",
	       regsr, cfg, serdes_get_number(sd, cfg));
	printf("           this is what the Service Processor latched before the PBI ran,\n");
	printf("           and what fsl_serdes_init() built its DPMAC map from.\n");
}

#if defined(CONFIG_ARCH_LX2160A)

struct sd1_mode {
	const char *name;			/* hwconfig serdes1:<name> */
	const char *desc;
	const char *from;
	bool (*is_active)(unsigned long base);
	int (*enter)(unsigned long base);
};

/*
 * 1x100GE + 4x25G
 *
 * This is NXP's e100g1_split.rcw sequence, moved from the PBI into
 * U-Boot.
 */
#define SD1_LNA_LND		GENMASK(3, 0)
#define SD1_LNE_LNH		GENMASK(7, 4)

/*
 * PROTO_SEL=11010b (25G/50G/100G, shared) + IF_WIDTH=100b (40-bit),
 * PORT_RST_LEFT=0 and PORT_LN0_B=0 so every lane becomes its own
 * single-lane port. Only those last two bits actually change: the
 * protocol and width are already what CAUI-4 was using.
 */
#define SPLIT_GCR0		0x000000D4
#define SPLIT_PCCE		0x00000100	/* E100Ga off, E100Gb on   */
#define SPLIT_PCCD		0x11110000	/* E25Ga..E25Gd on         */

#define SRDS_POLL_US		10000		/* NXP waits ~100 cycles   */

static bool sd1_is_100g4x25g(unsigned long base)
{
	u32 pccd = in_le32((void __iomem *)SRDS_PCCD(base));
	u32 pcce = in_le32((void __iomem *)SRDS_PCCE(base));

	return PCCE_E100GA_CFG(pcce) == 0 &&
	       PCCD_E25G_CFG(pccd, 0) && PCCD_E25G_CFG(pccd, 1) &&
	       PCCD_E25G_CFG(pccd, 2) && PCCD_E25G_CFG(pccd, 3);
}

static bool sd1_is_2x100g(unsigned long base)
{
	u32 pccd = in_le32((void __iomem *)SRDS_PCCD(base));
	u32 pcce = in_le32((void __iomem *)SRDS_PCCE(base));

	return PCCE_E100GA_CFG(pcce) != 0 && PCCE_E100GB_CFG(pcce) != 0 &&
	       PCCD_E25G_CFG(pccd, 0) == 0 && PCCD_E25G_CFG(pccd, 1) == 0 &&
	       PCCD_E25G_CFG(pccd, 2) == 0 && PCCD_E25G_CFG(pccd, 3) == 0;
}

/*
 * Wait for a self-clearing request bit. Per the doc, hardware
 * clears HLT_REQ when the halt completes and RST_REQ when the reset
 * completes.
 */
static int poll_clear(unsigned long reg, u32 bit)
{
	int us = SRDS_POLL_US;

	while (us--) {
		if (!(in_le32((void __iomem *)reg) & bit))
			return 0;
		udelay(1);
	}

	return -ETIMEDOUT;
}

static int sd1_port_master(unsigned long base, unsigned long lanes)
{
	int m, master = -1;

	for_each_set_bit(m, &lanes, SRDS_LANES)
		if (!(in_le32((void __iomem *)SRDS_LNmGCR0(base, m)) &
		      GCR0_PORT_LN0_B))
			master = m;

	if (master < 0)
		printf("lx2160a serdes: no master lane among LN%c-LN%c, refusing\n",
		       'A' + (int)__ffs(lanes), 'A' + (int)__fls(lanes));

	return master;
}

/*
 * Halt the port made of @lanes.
 *
 * "Halt only has meaning on the master source clock lane of a port
 * (LNmGCR0[PORT_LN0_B]=0)", so the request goes to every lane, as
 * NXP's sequence does, and only the master is waited on:
 * it is the one lane where the bit can clear.
 *
 *  -EINVAL means nothing was touched
 *  -ETIMEDOUT is only a warning
 */
static int sd1_halt_port(unsigned long base, unsigned long lanes)
{
	int m, master = sd1_port_master(base, lanes);

	if (master < 0)
		return -EINVAL;

	for_each_set_bit(m, &lanes, SRDS_LANES) {
		out_le32((void __iomem *)SRDS_LNmTRSTCTL(base, m),
			 RSTCTL_HLT_REQ);
		out_le32((void __iomem *)SRDS_LNmRRSTCTL(base, m),
			 RSTCTL_HLT_REQ);
	}

	if (poll_clear(SRDS_LNmTRSTCTL(base, master), RSTCTL_HLT_REQ) ||
	    poll_clear(SRDS_LNmRRSTCTL(base, master), RSTCTL_HLT_REQ)) {
		printf("lx2160a serdes: LN%c did not halt within %d us, continuing\n",
		       'A' + master, SRDS_POLL_US);
		return -ETIMEDOUT;
	}

	return 0;
}

static void sd1_reset_req(unsigned long base, unsigned long lanes)
{
	int m;

	for_each_set_bit(m, &lanes, SRDS_LANES) {
		out_le32((void __iomem *)SRDS_LNmTRSTCTL(base, m),
			 RSTCTL_RST_REQ);
		out_le32((void __iomem *)SRDS_LNmRRSTCTL(base, m),
			 RSTCTL_RST_REQ);
	}
}

static int sd1_reset_wait(unsigned long base, unsigned long lanes)
{
	int m, ret = 0;

	for_each_set_bit(m, &lanes, SRDS_LANES) {
		u32 t, r;

		if (poll_clear(SRDS_LNmTRSTCTL(base, m), RSTCTL_RST_REQ) ||
		    poll_clear(SRDS_LNmRRSTCTL(base, m), RSTCTL_RST_REQ)) {
			printf("lx2160a serdes: LN%c reset did not complete within %d us\n",
			       'A' + m, SRDS_POLL_US);
			ret = -ETIMEDOUT;
			continue;
		}

		t = in_le32((void __iomem *)SRDS_LNmTRSTCTL(base, m));
		r = in_le32((void __iomem *)SRDS_LNmRRSTCTL(base, m));
		if (!(t & RSTCTL_RST_DONE) || !(r & RSTCTL_RST_DONE)) {
			printf("lx2160a serdes: LN%c reset finished without RST_DONE (TRSTCTL=%08x RRSTCTL=%08x)\n",
			       'A' + m, t, r);
			ret = -EIO;
		}
	}

	return ret;
}

static int sd1_enter_100g4x25g(unsigned long base)
{
	unsigned long lanes = SD1_LNE_LNH;
	int m, ret, err;

	/* Lanes E-H are one 100G port at this point. */
	ret = sd1_halt_port(base, lanes);
	if (ret == -EINVAL)
		return ret;

	/*
	 * Reconfigure while halted: four one-lane ports instead of one
	 * four-lane port. Everything else about the lane is unchanged.
	 */
	for_each_set_bit(m, &lanes, SRDS_LANES)
		out_le32((void __iomem *)SRDS_LNmGCR0(base, m), SPLIT_GCR0);

	/* Re-point the protocol converters. */
	out_le32((void __iomem *)SRDS_PCCE(base), SPLIT_PCCE);
	out_le32((void __iomem *)SRDS_PCCD(base), SPLIT_PCCD);

	/* Reset. Every lane is its own master now, so all four count. */
	sd1_reset_req(base, lanes);
	err = sd1_reset_wait(base, lanes);

	return err ? err : ret;
}

/*
 * 2x40GE (XLAUI-4), dpmac.2 on LNA-LND and dpmac.1 on LNE-LNH,
 * protocol 20 builds the same.
 *
 * A PLL makes one clock net, and each lane takes its rate from the
 * clock net of one PLL, through its own BY_N_RATE_SEL. 25GE and 100GE
 * lanes run at 25.78125 Gbaud, twice the 12.890625 GHz clock net of
 * PLLF. 40GE lanes run at 10.3125 Gbaud, which no ratio of 12.890625
 * GHz gives: they need the 10.3125 GHz clock net, which only PLLS
 * makes (PLLnCR1[FRATE_SEL]=00110b). So the lanes move to PLLS.
 *
 * Protocol 13 does not use PLLS, and NXP's RCWs for it power it down
 * (SRDS_PLL_PD_PLL2=1), which leaves PLLSRSTCTL[DIS] set: software can
 * clear it. PLLS then gets the PLLSCR0/CR1/CR5 values the SoC loads
 * for protocol 20 when SRDS_PLL_REF_CLK_SEL_S1 selects 161.1328125 MHz
 * for PLLS.
 *
 * Only a 161.1328125 MHz PLLS reference clock is supported. It comes
 * from the SD1 PLLS reference clock input, or from the PLLF one when
 * the RCW sets SRDS_INTRA_REF_CLK_S1=1, as 25GE and 100GE already need
 * 161.1328125 MHz on PLLF. 156.25 MHz, also valid for a 10.3125 GHz
 * clock net, would need PLLSCR0[REFCLK_SEL]=00010b and is not handled.
 * Nothing here can check the reference clock: PLLSRSTCTL[LOCK] does
 * not tell its frequency.
 */
#define SRDS_PLLF		0
#define SRDS_PLLS		1
#define SLOW_PLLS_CR0		0x00040000	/* REFCLK_SEL=00100b, 161.1328125 MHz */
#define SLOW_PLLS_CR1		0x86100008	/* 10.3125 GHz clock net */
#define SLOW_PLLS_CR5		0x00001000
/* PROTO_SEL=01010b (XFI/10GBase-R/40GBase-R) + IF_WIDTH=010b (20-bit) */
#define F40G_GCR0_MASTER	0x00000052
#define F40G_GCR0_SLAVE		0x00010052
/* LNmTGCR0/LNmRGCR0: USE_SLOW_PLL=1 (bit 28), BY_N_RATE_SEL=000b full */
#define SLOW_LANE_GCR		0x10000000
#define F40G_PCCE		0x11000000	/* E40Ga+E40Gb on, E100G off */

static bool sd1_is_2x40g(unsigned long base)
{
	u32 pcce = in_le32((void __iomem *)SRDS_PCCE(base));

	return PCCE_E40GA_CFG(pcce) != 0 && PCCE_E40GB_CFG(pcce) != 0 &&
	       PCCE_E100GA_CFG(pcce) == 0 && PCCE_E100GB_CFG(pcce) == 0;
}

/* Program PLLS (Slow) for 10.3125 GHz, and wait for its lock */
static int sd1_plls_up(unsigned long base)
{
	unsigned long rstctl = SRDS_PLLnRSTCTL(base, SRDS_PLLS);
	u32 v;

	out_le32((void __iomem *)SRDS_PLLnCR0(base, SRDS_PLLS), SLOW_PLLS_CR0);
	out_le32((void __iomem *)SRDS_PLLnCR1(base, SRDS_PLLS), SLOW_PLLS_CR1);
	out_le32((void __iomem *)SRDS_PLLnCR5(base, SRDS_PLLS), SLOW_PLLS_CR5);

	out_le32((void __iomem *)rstctl, 0);		/* clears DIS */
	out_le32((void __iomem *)rstctl, RSTCTL_RST_REQ);

	if (poll_clear(rstctl, RSTCTL_RST_REQ)) {
		printf("lx2160a serdes: SD1 PLLS reset did not complete within %d us\n",
		       SRDS_POLL_US);
		return -ETIMEDOUT;
	}

	v = in_le32((void __iomem *)rstctl);
	if ((v & PLLRSTCTL_RST_ERR) || !(v & PLLRSTCTL_LOCK) ||
	    !(v & RSTCTL_RST_DONE)) {
		printf("lx2160a serdes: SD1 PLLS did not lock (PLLSRSTCTL=%08x)\n",
		       v);
		return -EIO;
	}

	return 0;
}

static int sd1_enter_2x40g(unsigned long base)
{
	unsigned long pllf = SRDS_PLLnRSTCTL(base, SRDS_PLLF);
	unsigned long plls = SRDS_PLLnRSTCTL(base, SRDS_PLLS);
	unsigned long lanes = SD1_LNA_LND | SD1_LNE_LNH;
	int m, lo, hi, ret, err;

	/* The two ports keep their master lanes. */
	lo = sd1_port_master(base, SD1_LNA_LND);
	hi = sd1_port_master(base, SD1_LNE_LNH);
	if (lo < 0 || hi < 0)
		return -EINVAL;

	ret = sd1_plls_up(base);
	if (ret)
		return ret;

	/*
	 * Halt PLLF, which puts every lane using it, all eight, in reset
	 * "to allow reconfiguration".
	 */
	out_le32((void __iomem *)pllf, RSTCTL_HLT_REQ);
	if (poll_clear(pllf, RSTCTL_HLT_REQ)) {
		printf("lx2160a serdes: SD1 PLLF did not halt within %d us, continuing\n",
		       SRDS_POLL_US);
		ret = -ETIMEDOUT;
	}

	for_each_set_bit(m, &lanes, SRDS_LANES) {
		out_le32((void __iomem *)SRDS_LNmGCR0(base, m),
			 m == lo || m == hi ? F40G_GCR0_MASTER : F40G_GCR0_SLAVE);
		out_le32((void __iomem *)SRDS_LNmTGCR0(base, m), SLOW_LANE_GCR);
		out_le32((void __iomem *)SRDS_LNmRGCR0(base, m), SLOW_LANE_GCR);
	}

	out_le32((void __iomem *)SRDS_PCCE(base), F40G_PCCE);

	/* Every lane now selects PLLS, so one PLL reset restarts them all. */
	out_le32((void __iomem *)plls, RSTCTL_RST_REQ);
	if (poll_clear(plls, RSTCTL_RST_REQ)) {
		printf("lx2160a serdes: SD1 PLLS reset did not complete within %d us\n",
		       SRDS_POLL_US);
		ret = -ETIMEDOUT;
	}

	err = sd1_reset_wait(base, lanes);

	return err ? err : ret;
}

/*
 * 1x40GE + 4x10G: dpmac.2 stays 40GE on LNA-LND, LNE-LNH become
 * 4xXFI ports, dpmac.3-6.
 * An XFI lane is a 40GE lane that is its own port master:
 * same PROTO_SEL and width, same PLLS and rate.
 *
 *   XFI   LNmGCR0 = 0x00000052   LNmTGCR0 = 0x10000000
 *   40GE  LNmGCR0 = 0x00010052   LNmTGCR0 = 0x10000000  (but its master)
 *                        ^ PORT_LN0_B
 */
#define XFI_GCR0		0x00000052	/* as 40GE, but own port master */
#define X10G_PCCE		0x01000000	/* E40Gb kept (lanes 0-3) */
#define XFI_PCCC		0x88889999	/* XFI converters for lanes 4-7 */

static bool sd1_is_40g4x10g(unsigned long base)
{
	u32 pccc = in_le32((void __iomem *)SRDS_PCCC(base));
	u32 pcce = in_le32((void __iomem *)SRDS_PCCE(base));

	return PCCE_E40GB_CFG(pcce) != 0 && PCCE_E40GA_CFG(pcce) == 0 &&
	       (pccc & 0xffff) == 0x9999;
}

static int sd1_enter_40g4x10g(unsigned long base)
{
	unsigned long lanes = SD1_LNE_LNH;
	int m, ret, err;

	ret = sd1_halt_port(base, lanes);
	if (ret == -EINVAL)
		return ret;

	for_each_set_bit(m, &lanes, SRDS_LANES)
		out_le32((void __iomem *)SRDS_LNmGCR0(base, m), XFI_GCR0);

	out_le32((void __iomem *)SRDS_PCCE(base), X10G_PCCE);
	out_le32((void __iomem *)SRDS_PCCC(base), XFI_PCCC);

	sd1_reset_req(base, lanes);
	err = sd1_reset_wait(base, lanes);

	return err ? err : ret;
}

/*
 * 1x100GE + 4x10G: dpmac.2 stays 100GE on LNA-LND at 25.78125 Gbaud on PLLF,
 * and LNE-LNH become four XFI ports, dpmac.3-6, at 10.3125 Gbaud on PLLS.
 *
 * Note: it is the only mode that runs SD1 on both PLLs Slow and Fast !
 */
#define M100G_PCCE		SPLIT_PCCE	/* E100Gb kept (lanes 0-3) */

static bool sd1_is_100g4x10g(unsigned long base)
{
	u32 pccc = in_le32((void __iomem *)SRDS_PCCC(base));
	u32 pcce = in_le32((void __iomem *)SRDS_PCCE(base));

	return PCCE_E100GB_CFG(pcce) != 0 && PCCE_E100GA_CFG(pcce) == 0 &&
	       (pccc & 0xffff) == 0x9999;
}

static int sd1_enter_100g4x10g(unsigned long base)
{
	unsigned long lanes = SD1_LNE_LNH;
	int m, ret, err;

	ret = sd1_plls_up(base);
	if (ret)
		return ret;

	/* Halt only the port that moves. */
	ret = sd1_halt_port(base, lanes);
	if (ret == -EINVAL)
		return ret;

	for_each_set_bit(m, &lanes, SRDS_LANES) {
		out_le32((void __iomem *)SRDS_LNmGCR0(base, m), XFI_GCR0);
		out_le32((void __iomem *)SRDS_LNmTGCR0(base, m), SLOW_LANE_GCR);
		out_le32((void __iomem *)SRDS_LNmRGCR0(base, m), SLOW_LANE_GCR);
	}

	out_le32((void __iomem *)SRDS_PCCE(base), M100G_PCCE);
	out_le32((void __iomem *)SRDS_PCCC(base), XFI_PCCC);

	/* As in 40g4x10g, only the lanes that moved are reset. */
	sd1_reset_req(base, lanes);
	err = sd1_reset_wait(base, lanes);

	return err ? err : ret;
}

static const struct sd1_mode sd1_modes[] = {
	{
		.name = "2x100g",
		.desc = "2x 100GE",
		.is_active = sd1_is_2x100g,
	},
	{
		.name = "100g4x25g",
		.desc = "1x 100GE + 4x 25G",
		.from = "2x100g",
		.is_active = sd1_is_100g4x25g,
		.enter = sd1_enter_100g4x25g,
	},
	{
		.name = "2x40g",
		.desc = "2x 40GE",
		.from = "2x100g",
		.is_active = sd1_is_2x40g,
		.enter = sd1_enter_2x40g,
	},
	{
		.name = "40g4x10g",
		.desc = "1x 40GE + 4x 10G",
		.from = "2x40g",
		.is_active = sd1_is_40g4x10g,
		.enter = sd1_enter_40g4x10g,
	},
	{
		.name = "100g4x10g",
		.desc = "1x 100GE + 4x 10G",
		.from = "2x100g",
		.is_active = sd1_is_100g4x10g,
		.enter = sd1_enter_100g4x10g,
	},
};

static const struct sd1_mode *sd1_mode_find(const char *name, size_t len)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sd1_modes); i++)
		if (strlen(sd1_modes[i].name) == len &&
		    !strncmp(sd1_modes[i].name, name, len))
			return &sd1_modes[i];

	return NULL;
}

static void sd1_print_pcc(unsigned long base)
{
	printf("PCCC=%08x PCCD=%08x PCCE=%08x",
	       in_le32((void __iomem *)SRDS_PCCC(base)),
	       in_le32((void __iomem *)SRDS_PCCD(base)),
	       in_le32((void __iomem *)SRDS_PCCE(base)));
}

static int sd1_enter(const struct sd1_mode *mode,
		     const struct sd1_mode *target, int depth)
{
	unsigned long base = srds_base(FSL_SRDS_1);
	const struct sd1_mode *from = NULL;
	int ret;

	if (mode->is_active(base))
		return 0;

	if (!mode->enter) {
		printf("lx2160a serdes: serdes1:%s needs SD1 to be %s, it is not (",
		       target->name, mode->desc);
		sd1_print_pcc(base);
		printf("), refusing\n");
		return -EINVAL;
	}

	if (mode->from)
		from = sd1_mode_find(mode->from, strlen(mode->from));
	if (!from || depth >= ARRAY_SIZE(sd1_modes))
		return -EINVAL;

	ret = sd1_enter(from, target, depth + 1);
	if (ret)
		return ret;

	ret = mode->enter(base);
	if (!mode->is_active(base)) {
		printf("lx2160a serdes: SD1 is not %s (", mode->desc);
		sd1_print_pcc(base);
		printf("), stopping\n");
		return ret ? ret : -EIO;
	}

	printf("lx2160a serdes: SD1 now %s%s\n", mode->desc,
	       ret ? " (with warnings above)" : "");

	return 0;
}

/*
 *   setenv hwconfig serdes1:<mode>
 *
 * Idempotent: a board whose PBI already configured SD1 skips it.
 */
void lx2160a_serdes_apply_hwconfig(void)
{
	const struct sd1_mode *mode;
	const char *arg;
	size_t len;

	arg = hwconfig_arg("serdes1", &len);
	if (!arg)
		return;

	mode = sd1_mode_find(arg, len);
	if (!mode) {
		printf("lx2160a serdes: unknown hwconfig serdes1:%.*s, ignored\n",
		       (int)len, arg);
		return;
	}

	if (mode->is_active(srds_base(FSL_SRDS_1)))
		return;

	if (IS_ENABLED(CONFIG_FSL_MC_ENET) && get_mc_boot_status() != -1) {
		printf("lx2160a serdes: the MC is already running, serdes1:%s not applied\n",
		       mode->name);
		return;
	}

	sd1_enter(mode, mode, 0);
}

#endif /* CONFIG_ARCH_LX2160A */

static int do_serdes_status(int sd)
{
	unsigned long base = srds_base(sd);
	int m;

	printf("SerDes %d @ 0x%08lx\n", sd + 1, base);
	print_rcw_view(sd);
	print_grouping(base);
	print_converters(base);
	for (m = 0; m < SRDS_LANES; m++)
		print_lane(base, m);

	return CMD_RET_SUCCESS;
}

static int do_lx2160_serdes(struct cmd_tbl *cmdtp, int flag,
			    int argc, char *const argv[])
{
	int sd;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "status"))
		return CMD_RET_USAGE;

	if (argc == 2) {
		for (sd = FSL_SRDS_1; sd < SRDS_COUNT; sd++) {
			do_serdes_status(sd);
			if (sd != SRDS_COUNT - 1)
				printf("\n");
		}
		return CMD_RET_SUCCESS;
	}

	sd = (int)dectoul(argv[2], NULL) - 1;
	if (sd < FSL_SRDS_1 || sd >= SRDS_COUNT) {
		printf("SerDes must be 1, 2 or 3\n");
		return CMD_RET_FAILURE;
	}

	return do_serdes_status(sd);
}

U_BOOT_CMD(
	lx2160_serdes, 3, 0, do_lx2160_serdes,
	"inspect the LX2160A 28G-Lynx SerDes protocol configuration",
	"status [1|2|3]\n"
	"    - print the live protocol converters (PCCC/PCCD/PCCE) and the\n"
	"      per-lane port grouping and protocol (LNmGCR0), next to the\n"
	"      protocol number the RCW latched.\n"
	"      No argument prints all three SerDes."
);
