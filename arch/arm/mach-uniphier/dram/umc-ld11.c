/*
 * Copyright (C) 2016 Socionext Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/processor.h>

#include "../init.h"
#include "ddrphy-regs.h"
#include "umc64-regs.h"

#define DDR_FREQ		1600

#define DRAM_CH_NR	2
#define RANK_BLOCKS_TR	2

enum dram_freq {
	DRAM_FREQ_1600M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

/* PHY */
static const int rof_pos_shift_pre[RANK_BLOCKS_TR][2] = { {0, 0}, {0, 0} };
static const int rof_neg_shift_pre[RANK_BLOCKS_TR][2] = { {0, 0}, {0, 0} };
static const int rof_pos_shift[RANK_BLOCKS_TR][2] = { {-35, -35}, {-35, -35} };
static const int rof_neg_shift[RANK_BLOCKS_TR][2] = { {-17, -17}, {-17, -17} };
static const int tof_shift[RANK_BLOCKS_TR][2] = { {-50, -50}, {-50, -50} };

/* Register address */
#define PHY_ZQ0CR1	0x00000184
#define PHY_ZQ1CR1	0x00000194
#define PHY_ZQ2CR1	0x000001A4
#define PHY_DX0GCR	0x000001C0
#define PHY_DX0GTR	0x000001F0
#define PHY_DX1GCR	0x00000200
#define PHY_DX1GTR	0x00000230
#define PHY_DX2GCR	0x00000240
#define PHY_DX2GTR	0x00000270
#define PHY_DX3GCR	0x00000280
#define PHY_DX3GTR	0x000002B0

#define PHY_DXMDLR(dx)		(0x000001EC + 0x40 * (dx))
#define PHY_DXLCDLR0(dx)	(0x000001E0 + 0x40 * (dx))
#define PHY_DXLCDLR1(dx)	(0x000001E4 + 0x40 * (dx))
#define PHY_DXLCDLR2(dx)	(0x000001E8 + 0x40 * (dx))
#define PHY_DXBDLR1(dx)		(0x000001D0 + 0x40 * (dx))
#define PHY_DXBDLR2(dx)		(0x000001D4 + 0x40 * (dx))

/* MASK */
#define PHY_ACBD_MASK		0x00FC0000
#define PHY_CK0BD_MASK		0x0000003F
#define PHY_CK1BD_MASK		0x00000FC0
#define PHY_IPRD_MASK		0x000000FF
#define PHY_WLD_MASK(rank)	(0xFF << (8 * (rank)))
#define PHY_DQSGD_MASK(rank)	(0xFF << (8 * (rank)))
#define PHY_DQSGX_MASK		BIT(6)
#define PHY_DSWBD_MASK		0x3F000000	/* bit[29:24] */
#define PHY_DSDQOE_MASK		0x00000FFF

static void ddrphy_maskwritel(u32 data, u32 mask, void __iomem *addr)
{
	u32 value;

	value = (readl(addr) & ~(mask)) | (data & mask);
	writel(value, addr);
}

static u32 ddrphy_maskreadl(u32 mask, void __iomem *addr)
{
	return readl(addr) & mask;
}

/* step of 0.5T  for PUB-byte */
static u8 ddrphy_get_mdl(int dx, void __iomem *phy_base)
{
	return ddrphy_maskreadl(PHY_IPRD_MASK, phy_base + PHY_DXMDLR(dx));
}

/* Calculating step for PUB-byte */
static int ddrphy_hpstep(int delay, int dx, void __iomem *phy_base)
{
	return delay * ddrphy_get_mdl(dx, phy_base) * DDR_FREQ / 1000000;
}

static void ddrphy_vt_ctrl(void __iomem *phy_base, int enable)
{
	u32 tmp;

	tmp = readl(phy_base + PHY_PGCR1);

	if (enable)
		tmp &= ~PHY_PGCR1_INHVT;
	else
		tmp |= PHY_PGCR1_INHVT;

	writel(tmp, phy_base + PHY_PGCR1);

	if (!enable) {
		while (!(readl(phy_base + PHY_PGSR1) & PHY_PGSR1_VTSTOP))
			cpu_relax();
	}
}

static void ddrphy_set_ckoffset_qoffset(int delay_ckoffset0, int delay_ckoffset1,
					int delay_qoffset, int enable,
					void __iomem *phy_base)
{
	u8 ck_step0, ck_step1;	/* ckoffset_step for clock */
	u8 q_step;	/*  qoffset_step for clock */
	int dx;

	dx = 2; /* use dx2 in sLD11 */

	ck_step0 = ddrphy_hpstep(delay_ckoffset0, dx, phy_base);     /* CK-Offset */
	ck_step1 = ddrphy_hpstep(delay_ckoffset1, dx, phy_base);     /* CK-Offset */
	q_step = ddrphy_hpstep(delay_qoffset, dx, phy_base);     /*  Q-Offset */

	ddrphy_vt_ctrl(phy_base, 0);

	/* Q->[23:18], CK1->[11:6], CK0->bit[5:0] */
	if (enable == 1)
		ddrphy_maskwritel((q_step << 18) + (ck_step1 << 6) + ck_step0,
				  PHY_ACBD_MASK | PHY_CK1BD_MASK | PHY_CK0BD_MASK,
				  phy_base + PHY_ACBDLR);

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_wl_delay_dx(int dx, int r0_delay, int r1_delay,
				   int enable, void __iomem *phy_base)
{
	int rank;
	int delay_wl[4];
	u32 wl_mask  = 0;   /* WriteLeveling's Mask  */
	u32 wl_value = 0;   /* WriteLeveling's Value */

	delay_wl[0] = r0_delay & 0xfff;
	delay_wl[1] = r1_delay & 0xfff;
	delay_wl[2] = 0;
	delay_wl[3] = 0;

	ddrphy_vt_ctrl(phy_base, 0);

	for (rank = 0; rank < 4; rank++) {
		wl_mask  |= PHY_WLD_MASK(rank);
		/*  WriteLeveling's delay */
		wl_value |= ddrphy_hpstep(delay_wl[rank], dx, phy_base) << (8 * rank);
	}

	if (enable == 1)
		ddrphy_maskwritel(wl_value, wl_mask, phy_base + PHY_DXLCDLR0(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_dqsg_delay_dx(int dx, int r0_delay, int r1_delay,
				     int enable, void __iomem *phy_base)
{
	int rank;
	int delay_dqsg[4];
	u32 dqsg_mask  = 0;   /* DQSGating_LCDL_delay's Mask  */
	u32 dqsg_value = 0;   /* DQSGating_LCDL_delay's Value */

	delay_dqsg[0] = r0_delay;
	delay_dqsg[1] = r1_delay;
	delay_dqsg[2] = 0;
	delay_dqsg[3] = 0;

	ddrphy_vt_ctrl(phy_base, 0);

	for (rank = 0; rank < 4; rank++)  {
		dqsg_mask  |= PHY_DQSGD_MASK(rank);
		 /* DQSGating's delay */
		dqsg_value |= ddrphy_hpstep(delay_dqsg[rank], dx, phy_base) << (8 * rank);
	}

	if (enable == 1)
		ddrphy_maskwritel(dqsg_value, dqsg_mask, phy_base + PHY_DXLCDLR2(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_dswb_delay_dx(int dx, int delay, int enable, void __iomem *phy_base)
{
	u8 dswb_step;

	ddrphy_vt_ctrl(phy_base, 0);

	dswb_step = ddrphy_hpstep(delay, dx, phy_base);     /* DQS-BDL's delay */

	if (enable == 1)
		ddrphy_maskwritel(dswb_step << 24, PHY_DSWBD_MASK, phy_base + PHY_DXBDLR1(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_oe_delay_dx(int dx, int dqs_delay, int dq_delay,
				   int enable, void __iomem *phy_base)
{
	u8 dqs_oe_step, dq_oe_step;
	u32 wdata;

	ddrphy_vt_ctrl(phy_base, 0);

	/* OE(DQS,DQ) */
	dqs_oe_step = ddrphy_hpstep(dqs_delay, dx, phy_base);     /* DQS-oe's delay */
	dq_oe_step = ddrphy_hpstep(dq_delay, dx, phy_base);     /* DQ-oe's delay */
	wdata = ((dq_oe_step<<6) + dqs_oe_step) & 0xFFF;

	if (enable == 1)
		ddrphy_maskwritel(wdata, PHY_DSDQOE_MASK, phy_base + PHY_DXBDLR2(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_ext_dqsgt(void __iomem *phy_base)
{
	/* Extend DQSGating_window   min:+1T  max:+1T */
	ddrphy_maskwritel(PHY_DQSGX_MASK, PHY_DQSGX_MASK, phy_base + PHY_DSGCR);
}

static void ddrphy_shift_tof_hws(void __iomem *phy_base, const int shift[][2])
{
	int dx, block, byte;
	u32 lcdlr1, wdqd;

	ddrphy_vt_ctrl(phy_base, 0);

	for (block = 0; block < RANK_BLOCKS_TR; block++) {
		for (byte = 0; byte < 2; byte++) {
			dx = block * 2 + byte;
			lcdlr1 = readl(phy_base + PHY_DXLCDLR1(dx));
			wdqd = lcdlr1 & 0xff;
			wdqd = clamp(wdqd + ddrphy_hpstep(shift[block][byte], dx, phy_base),
				     0U, 0xffU);
			lcdlr1 = (lcdlr1 & ~0xff) | wdqd;
			writel(lcdlr1, phy_base + PHY_DXLCDLR1(dx));
			readl(phy_base + PHY_DXLCDLR1(dx)); /* relax */
		}
	}

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_shift_rof_hws(void __iomem *phy_base, const int pos_shift[][2],
				 const int neg_shift[][2])
{
	int dx, block, byte;
	u32 lcdlr1, rdqsd, rdqnsd;

	ddrphy_vt_ctrl(phy_base, 0);

	for (block = 0; block < RANK_BLOCKS_TR; block++) {
		for (byte = 0; byte < 2; byte++) {
			dx = block * 2 + byte;
			lcdlr1 = readl(phy_base + PHY_DXLCDLR1(dx));

			/*  DQS LCDL  RDQNSD->[23:16]  RDQSD->[15:8] */
			rdqsd  = (lcdlr1 >> 8) & 0xff;
			rdqnsd = (lcdlr1 >> 16) & 0xff;
			rdqsd  = clamp(rdqsd + ddrphy_hpstep(pos_shift[block][byte], dx, phy_base),
				       0U, 0xffU);
			rdqnsd = clamp(rdqnsd + ddrphy_hpstep(neg_shift[block][byte], dx, phy_base),
				       0U, 0xffU);
			lcdlr1 = (lcdlr1 & ~(0xffff << 8)) | (rdqsd << 8) | (rdqnsd << 16);
			writel(lcdlr1, phy_base + PHY_DXLCDLR1(dx));
			readl(phy_base + PHY_DXLCDLR1(dx)); /* relax */
		}
	}

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_boot_run_hws(void __iomem *phy_base)
{
	/* Hard Training for DIO */
	writel(0x0000f401, phy_base + PHY_PIR);
	while (!(readl(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		cpu_relax();
}

static void ddrphy_training(void __iomem *phy_base)
{
	/* DIO roffset shift before hard training */
	ddrphy_shift_rof_hws(phy_base, rof_pos_shift_pre, rof_neg_shift_pre);

	/* Hard Training for each CH */
	ddrphy_boot_run_hws(phy_base);

	/* DIO toffset shift after training */
	ddrphy_shift_tof_hws(phy_base, tof_shift);

	/* DIO roffset shift after training */
	ddrphy_shift_rof_hws(phy_base, rof_pos_shift, rof_neg_shift);

	/* Extend DQSGating window  min:+1T  max:+1T */
	ddrphy_ext_dqsgt(phy_base);
}

static void ddrphy_init(void __iomem *phy_base, enum dram_freq freq)
{
	writel(0x40000000, phy_base + PHY_PIR);
	writel(0x0300C4F1, phy_base + PHY_PGCR1);
	writel(0x0C807D04, phy_base + PHY_PTR0);
	writel(0x27100578, phy_base + PHY_PTR1);
	writel(0x00083DEF, phy_base + PHY_PTR2);
	writel(0x12061A80, phy_base + PHY_PTR3);
	writel(0x08027100, phy_base + PHY_PTR4);
	writel(0x9D9CBB66, phy_base + PHY_DTPR0);
	writel(0x1a878400, phy_base + PHY_DTPR1);
	writel(0x50025200, phy_base + PHY_DTPR2);
	writel(0xF004641A, phy_base + PHY_DSGCR);
	writel(0x0000040B, phy_base + PHY_DCR);
	writel(0x00000d71, phy_base + PHY_MR0);
	writel(0x00000006, phy_base + PHY_MR1);
	writel(0x00000098, phy_base + PHY_MR2);
	writel(0x00000000, phy_base + PHY_MR3);

	while (!(readl(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		cpu_relax();

	writel(0x00000059, phy_base + PHY_ZQ0CR1);
	writel(0x00000019, phy_base + PHY_ZQ1CR1);
	writel(0x00000019, phy_base + PHY_ZQ2CR1);
	writel(0x30FC6C20, phy_base + PHY_PGCR2);

	ddrphy_set_ckoffset_qoffset(119, 0, 0, 1, phy_base);
	ddrphy_set_wl_delay_dx(0, 220, 220, 1, phy_base);
	ddrphy_set_wl_delay_dx(1, 160, 160, 1, phy_base);
	ddrphy_set_wl_delay_dx(2, 190, 190, 1, phy_base);
	ddrphy_set_wl_delay_dx(3, 150, 150, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(0, 750, 750, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(1, 750, 750, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(2, 750, 750, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(3, 750, 750, 1, phy_base);
	ddrphy_set_dswb_delay_dx(0, 0, 1, phy_base);
	ddrphy_set_dswb_delay_dx(1, 0, 1, phy_base);
	ddrphy_set_dswb_delay_dx(2, 0, 1, phy_base);
	ddrphy_set_dswb_delay_dx(3, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(0, 0, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(1, 0, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(2, 0, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(3, 0, 0, 1, phy_base);

	writel(0x44000E81, phy_base + PHY_DX0GCR);
	writel(0x44000E81, phy_base + PHY_DX1GCR);
	writel(0x44000E81, phy_base + PHY_DX2GCR);
	writel(0x44000E81, phy_base + PHY_DX3GCR);
	writel(0x00055002, phy_base + PHY_DX0GTR);
	writel(0x00055002, phy_base + PHY_DX1GTR);
	writel(0x00055010, phy_base + PHY_DX2GTR);
	writel(0x00055010, phy_base + PHY_DX3GTR);
	writel(0x930035C7, phy_base + PHY_DTCR);
	writel(0x00000003, phy_base + PHY_PIR);
	readl(phy_base + PHY_PIR);
	while (!(readl(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		cpu_relax();

	writel(0x00000181, phy_base + PHY_PIR);
	readl(phy_base + PHY_PIR);
	while (!(readl(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		cpu_relax();

	writel(0x44181884, phy_base + PHY_DXCCR);
	writel(0x00000001, phy_base + PHY_GPR1);
}

/* UMC */
static const u32 umc_cmdctla[DRAM_FREQ_NR] = {0x060B0B1C};
static const u32 umc_cmdctlb[DRAM_FREQ_NR] = {0x27201806};
static const u32 umc_cmdctlc[DRAM_FREQ_NR] = {0x00120B04};
static const u32 umc_cmdctle[DRAM_FREQ_NR] = {0x00680607};
static const u32 umc_cmdctlf[DRAM_FREQ_NR] = {0x02000200};
static const u32 umc_cmdctlg[DRAM_FREQ_NR] = {0x08080808};

static const u32 umc_rdatactl[DRAM_FREQ_NR] = {0x00000810};
static const u32 umc_wdatactl[DRAM_FREQ_NR] = {0x00000004};
static const u32 umc_odtctl[DRAM_FREQ_NR]   = {0x02000002};
static const u32 umc_acssetb[DRAM_CH_NR] = {0x00000200, 0x00000203};

static const u32 umc_memconfch[DRAM_FREQ_NR] = {0x00023605};

static int umc_dc_init(void __iomem *dc_base, enum dram_freq freq,
		       unsigned long size, int ch)
{
	/* Wait for PHY Init Complete */
	writel(umc_cmdctla[freq], dc_base + UMC_CMDCTLA);
	writel(umc_cmdctlb[freq], dc_base + UMC_CMDCTLB);
	writel(umc_cmdctlc[freq], dc_base + UMC_CMDCTLC);
	writel(umc_cmdctle[freq], dc_base + UMC_CMDCTLE);
	writel(umc_cmdctlf[freq], dc_base + UMC_CMDCTLF);
	writel(umc_cmdctlg[freq], dc_base + UMC_CMDCTLG);

	writel(umc_rdatactl[freq], dc_base + UMC_RDATACTL_D0);
	writel(umc_rdatactl[freq], dc_base + UMC_RDATACTL_D1);

	writel(umc_wdatactl[freq], dc_base + UMC_WDATACTL_D0);
	writel(umc_wdatactl[freq], dc_base + UMC_WDATACTL_D1);

	writel(umc_odtctl[freq], dc_base + UMC_ODTCTL_D0);
	writel(umc_odtctl[freq], dc_base + UMC_ODTCTL_D1);

	writel(0x00000003, dc_base + UMC_ACSSETA);
	writel(0x00000103, dc_base + UMC_FLOWCTLG);
	writel(umc_acssetb[ch], dc_base + UMC_ACSSETB);
	writel(0x02020200, dc_base + UMC_SPCSETB);
	writel(umc_memconfch[freq], dc_base + UMC_MEMCONFCH);
	writel(0x00000002, dc_base + UMC_ACFETCHCTRL);

	return 0;
}

static int umc_ch_init(void __iomem *umc_ch_base,
		       enum dram_freq freq, unsigned long size, int ch)
{
	void __iomem *dc_base  = umc_ch_base;

	return umc_dc_init(dc_base, freq, size, ch);
}

static void um_init(void __iomem *um_base)
{
	writel(0x00000001, um_base + UMC_SIORST);
	writel(0x00000001, um_base + UMC_VO0RST);
	writel(0x00000001, um_base + UMC_VPERST);
	writel(0x00000001, um_base + UMC_RGLRST);
	writel(0x00000001, um_base + UMC_A2DRST);
	writel(0x00000001, um_base + UMC_DMDRST);
}

int uniphier_ld11_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *um_base = (void __iomem *)0x5B800000;
	void __iomem *umc_ch_base = (void __iomem *)0x5BC00000;
	void __iomem *phy_base = (void __iomem *)0x5BC01000;
	enum dram_freq freq;
	int ch, ret;

	switch (bd->dram_freq) {
	case 1600:
		freq = DRAM_FREQ_1600M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", bd->dram_freq);
		return -EINVAL;
	}

	writel(0x00000101, umc_ch_base + UMC_DIOCTLA);
	while (!(readl(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		cpu_relax();

	writel(0x00000000, umc_ch_base + UMC_DIOCTLA);
	writel(0x00000001, umc_ch_base + UMC_DEBUGC);
	writel(0x00000101, umc_ch_base + UMC_DIOCTLA);

	writel(0x00000100, umc_ch_base + UMC_INITSET);
	while (readl(umc_ch_base + UMC_INITSTAT) & BIT(8))
		cpu_relax();

	writel(0x00000100, umc_ch_base + 0x00200000 + UMC_INITSET);
	while (readl(umc_ch_base + 0x00200000 + UMC_INITSTAT) & BIT(8))
		cpu_relax();

	ddrphy_init(phy_base, freq);

	for (ch = 0; ch < DRAM_CH_NR; ch++) {
		unsigned long size = bd->dram_ch[ch].size;
		unsigned int width = bd->dram_ch[ch].width;

		ret = umc_ch_init(umc_ch_base, freq, size / (width / 16), ch);
		if (ret) {
			pr_err("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		umc_ch_base += 0x00200000;
	}
	ddrphy_training(phy_base);

	um_init(um_base);

	return 0;
}
