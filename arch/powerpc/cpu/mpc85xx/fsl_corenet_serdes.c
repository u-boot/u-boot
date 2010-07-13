/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
#include <hwconfig.h>
#endif
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/fsl_law.h>
#include "fsl_corenet_serdes.h"

static u32 serdes_prtcl_map;

#ifdef DEBUG
static const char *serdes_prtcl_str[] = {
	[NONE] = "NA",
	[PCIE1] = "PCIE1",
	[PCIE2] = "PCIE2",
	[PCIE3] = "PCIE3",
	[PCIE4] = "PCIE4",
	[SATA1] = "SATA1",
	[SATA2] = "SATA2",
	[SRIO1] = "SRIO1",
	[SRIO2] = "SRIO2",
	[SGMII_FM1_DTSEC1] = "SGMII_FM1_DTSEC1",
	[SGMII_FM1_DTSEC2] = "SGMII_FM1_DTSEC2",
	[SGMII_FM1_DTSEC3] = "SGMII_FM1_DTSEC3",
	[SGMII_FM1_DTSEC4] = "SGMII_FM1_DTSEC4",
	[SGMII_FM1_DTSEC5] = "SGMII_FM1_DTSEC5",
	[SGMII_FM2_DTSEC1] = "SGMII_FM2_DTSEC1",
	[SGMII_FM2_DTSEC2] = "SGMII_FM2_DTSEC2",
	[SGMII_FM2_DTSEC3] = "SGMII_FM2_DTSEC3",
	[SGMII_FM2_DTSEC4] = "SGMII_FM2_DTSEC4",
	[XAUI_FM1] = "XAUI_FM1",
	[XAUI_FM2] = "XAUI_FM2",
	[AURORA] = "DEBUG",
};
#endif

static const struct {
	int idx;
	unsigned int lpd; /* RCW lane powerdown bit */
	int bank;
} lanes[SRDS_MAX_LANES] = {
	{ 0, 152, FSL_SRDS_BANK_1 },
	{ 1, 153, FSL_SRDS_BANK_1 },
	{ 2, 154, FSL_SRDS_BANK_1 },
	{ 3, 155, FSL_SRDS_BANK_1 },
	{ 4, 156, FSL_SRDS_BANK_1 },
	{ 5, 157, FSL_SRDS_BANK_1 },
	{ 6, 158, FSL_SRDS_BANK_1 },
	{ 7, 159, FSL_SRDS_BANK_1 },
	{ 8, 160, FSL_SRDS_BANK_1 },
	{ 9, 161, FSL_SRDS_BANK_1 },
	{ 16, 162, FSL_SRDS_BANK_2 },
	{ 17, 163, FSL_SRDS_BANK_2 },
	{ 18, 164, FSL_SRDS_BANK_2 },
	{ 19, 165, FSL_SRDS_BANK_2 },
	{ 20, 170, FSL_SRDS_BANK_3 },
	{ 21, 171, FSL_SRDS_BANK_3 },
	{ 22, 172, FSL_SRDS_BANK_3 },
	{ 23, 173, FSL_SRDS_BANK_3 },
};

int serdes_get_lane_idx(int lane)
{
	return lanes[lane].idx;
}

int serdes_get_bank(int lane)
{
	return lanes[lane].bank;
}

int serdes_lane_enabled(int lane)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes_corenet_t *regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;

	int bank = lanes[lane].bank;
	int word = lanes[lane].lpd / 32;
	int bit = lanes[lane].lpd % 32;

	if (in_be32(&regs->bank[bank].rstctl) & SRDS_RSTCTL_SDPD)
		return 0;

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	if (!IS_SVR_REV(get_svr(), 1, 0))
		if (bank > 0)
			return !(srds_lpd_b[bank] &
					(8 >> (lane - (6 + 4 * bank))));
#endif

	return !(in_be32(&gur->rcwsr[word]) & (0x80000000 >> bit));
}

int is_serdes_configured(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* Is serdes enabled at all? */
	if (!(in_be32(&gur->rcwsr[5]) & FSL_CORENET_RCWSR5_SRDS_EN))
		return 0;

	return (1 << device) & serdes_prtcl_map;
}

#ifndef CONFIG_SYS_DCSRBAR_PHYS
#define CONFIG_SYS_DCSRBAR_PHYS	0x80000000 /* Must be 1GB-aligned for rev1.0 */
#define CONFIG_SYS_DCSRBAR	0x80000000
#define __DCSR_NOT_DEFINED_BY_CONFIG
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
static void enable_bank(ccsr_gur_t *gur, int bank)
{
	u32 rcw5;

	/*
	 * Enable the lanes SRDS_LPD_Bn.  The RCW bits are read-only in
	 * CCSR, and read/write in DSCR.
	 */
	rcw5 = in_be32(gur->rcwsr + 5);
	if (bank == FSL_SRDS_BANK_2) {
		rcw5 &= ~FSL_CORENET_RCWSRn_SRDS_LPD_B2;
		rcw5 |= srds_lpd_b[bank] << 26;
	} else if (bank == FSL_SRDS_BANK_3) {
		rcw5 &= ~FSL_CORENET_RCWSRn_SRDS_LPD_B3;
		rcw5 |= srds_lpd_b[bank] << 18;
	} else {
		printf("SERDES: enable_bank: bad bank %d\n", bank + 1);
		return;
	}

	/* See similar code in cpu/mpc85xx/cpu_init.c for an explanation
	 * of the DCSR mapping.
	 */
	{
#ifdef __DCSR_NOT_DEFINED_BY_CONFIG
		struct law_entry law = find_law(CONFIG_SYS_DCSRBAR_PHYS);
		int law_index;
		if (law.index == -1)
			law_index = set_next_law(CONFIG_SYS_DCSRBAR_PHYS,
						 LAW_SIZE_1M, LAW_TRGT_IF_DCSR);
		else
			set_law(law.index, CONFIG_SYS_DCSRBAR_PHYS, LAW_SIZE_1M,
				LAW_TRGT_IF_DCSR);
#endif
		u32 *p = (void *)CONFIG_SYS_DCSRBAR + 0x20114;
		out_be32(p, rcw5);
#ifdef __DCSR_NOT_DEFINED_BY_CONFIG
		if (law.index == -1)
			disable_law(law_index);
		else
			set_law(law.index, law.addr, law.size, law.trgt_id);
#endif
	}
}

/*
 * To avoid problems with clock jitter, rev 2 p4080 uses the pll from
 * bank 3 to clock banks 2 and 3, as well as a limited selection of
 * protocol configurations.  This requires that banks 2 and 3's lanes be
 * disabled in the RCW, and enabled with some fixup here to re-enable
 * them, and to configure bank 2's clock parameters in bank 3's pll in
 * cases where they differ.
 */
static void p4080_erratum_serdes8(serdes_corenet_t *regs, ccsr_gur_t *gur,
				  u32 devdisr, u32 devdisr2, int cfg)
{
	int srds_ratio_b2;
	int rfck_sel;

	/*
	 * The disabled lanes of bank 2 will cause the associated
	 * logic blocks to be disabled in DEVDISR.  We reverse that here.
	 *
	 * Note that normally it is not permitted to clear DEVDISR bits
	 * once the device has been disabled, but the hardware people
	 * say that this special case is OK.
	 */
	clrbits_be32(&gur->devdisr, devdisr);
	clrbits_be32(&gur->devdisr2, devdisr2);

	/*
	 * Some protocols require special handling.  There are a few
	 * additional protocol configurations that can be used, which are
	 * not listed here.  See app note 4065 for supported protocol
	 * configurations.
	 */
	switch (cfg) {
	case 0x19:
		/*
		 * Bank 2 has PCIe which wants BWSEL -- tell bank 3's PLL.
		 * SGMII on bank 3 should still be usable.
		 */
		setbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr1,
			     SRDS_PLLCR1_PLL_BWSEL);

		enable_bank(gur, FSL_SRDS_BANK_3);
		break;

	case 0x0f:
	case 0x10:
		/*
		 * Banks 2 (XAUI) and 3 (SGMII) have different clocking
		 * requirements in these configurations.  Bank 3 cannot
		 * be used and should have its lanes (but not the bank
		 * itself) disabled in the RCW.  We set up bank 3's pll
		 * for bank 2's needs here.
		 */
		srds_ratio_b2 = (in_be32(&gur->rcwsr[4]) >> 13) & 7;

		/* Determine refclock from XAUI ratio */
		switch (srds_ratio_b2) {
		case 1: /* 20:1 */
			rfck_sel = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		case 2: /* 25:1 */
			rfck_sel = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		default:
			printf("SERDES: bad SRDS_RATIO_B2 %d\n",
			       srds_ratio_b2);
			return;
		}

		clrsetbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr0,
				SRDS_PLLCR0_RFCK_SEL_MASK, rfck_sel);

		clrsetbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr0,
				SRDS_PLLCR0_FRATE_SEL_MASK,
				SRDS_PLLCR0_FRATE_SEL_6_25);
		break;
	default:
		enable_bank(gur, FSL_SRDS_BANK_3);
	}

}
#endif

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int cfg;
	serdes_corenet_t *srds_regs;
	int lane, bank, idx;
	enum srds_prtcl lane_prtcl;
	long long end_tick;
	int have_bank[SRDS_MAX_BANK] = {};
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	u32 serdes8_devdisr = 0;
	u32 serdes8_devdisr2 = 0;
	char srds_lpd_opt[16];
	const char *srds_lpd_arg;
	size_t arglen;
#endif

	/* Is serdes enabled at all? */
	if (!(in_be32(&gur->rcwsr[5]) & FSL_CORENET_RCWSR5_SRDS_EN))
		return;

	srds_regs = (void *)(CONFIG_SYS_FSL_CORENET_SERDES_ADDR);
	cfg = (in_be32(&gur->rcwsr[4]) & FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;
	debug("Using SERDES configuration 0x%x, lane settings:\n", cfg);

	if (!is_serdes_prtcl_valid(cfg)) {
		printf("SERDES[PRTCL] = 0x%x is not valid\n", cfg);
		return;
	}

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	if (!IS_SVR_REV(get_svr(), 1, 0))
		for (bank = 1; bank < ARRAY_SIZE(srds_lpd_b); bank++) {
			sprintf(srds_lpd_opt, "fsl_srds_lpd_b%u", bank + 1);
			srds_lpd_arg = hwconfig_subarg("serdes", srds_lpd_opt,
						       &arglen);
			if (srds_lpd_arg)
				srds_lpd_b[bank] = simple_strtoul(srds_lpd_arg,
								  NULL, 0);
		}
#endif

	/* Look for banks with all lanes disabled, and power down the bank. */
	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(cfg, lane);
		if (serdes_lane_enabled(lane)) {
			have_bank[serdes_get_bank(lane)] = 1;
			serdes_prtcl_map |= (1 << lane_prtcl);
		}
	}

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	if (IS_SVR_REV(get_svr(), 1, 0)) {
		/* At least one bank must be disabled due to SERDES8.  If
		 * no bank is found to be disabled based on lane
		 * disables, disable bank 3 because we can't turn off its
		 * lanes in the RCW without disabling MDIO due to erratum
		 * GEN8.
		 *
		 * This means that if you are relying on bank 3 being
		 * disabled to avoid SERDES8, in some cases you cannot
		 * also disable all lanes of another bank, or else bank
		 * 3 won't be disabled, leaving you with a configuration
		 * that isn't valid according to SERDES8 (e.g. if banks
		 * 2 and 3 have the same clock, and bank 1 is disabled
		 * instead of 3).
		 */
		for (bank = 0; bank < SRDS_MAX_BANK; bank++) {
			if (!have_bank[bank])
				break;
		}

		if (bank == SRDS_MAX_BANK)
			have_bank[FSL_SRDS_BANK_3] = 0;
	} else {
		if (have_bank[FSL_SRDS_BANK_2])
			have_bank[FSL_SRDS_BANK_3] = 1;
	}
#endif

	for (bank = 0; bank < SRDS_MAX_BANK; bank++) {
		if (!have_bank[bank]) {
			printf("SERDES: bank %d disabled\n", bank + 1);
			setbits_be32(&srds_regs->bank[bank].rstctl,
				     SRDS_RSTCTL_SDPD);
		}
	}

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		idx = serdes_get_lane_idx(lane);
		lane_prtcl = serdes_get_prtcl(cfg, lane);

#ifdef DEBUG
		switch (lane) {
		case 0:
			puts("Bank1: ");
			break;
		case 10:
			puts("\nBank2: ");
			break;
		case 14:
			puts("\nBank3: ");
			break;
		default:
			break;
		}

		printf("%s ", serdes_prtcl_str[lane_prtcl]);
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
		switch (lane_prtcl) {
		case PCIE1:
		case PCIE2:
		case PCIE3:
			serdes8_devdisr |= FSL_CORENET_DEVDISR_PCIE1 >>
					   (lane_prtcl - PCIE1);
			break;
		case SRIO1:
		case SRIO2:
			serdes8_devdisr |= FSL_CORENET_DEVDISR_SRIO1 >>
					   (lane_prtcl - SRIO1);
			break;
		case SGMII_FM1_DTSEC1:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_1;
			break;
		case SGMII_FM1_DTSEC2:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_2;
			break;
		case SGMII_FM1_DTSEC3:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_3;
			break;
		case SGMII_FM1_DTSEC4:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_4;
			break;
		case SGMII_FM2_DTSEC1:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_1;
			break;
		case SGMII_FM2_DTSEC2:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_2;
			break;
		case SGMII_FM2_DTSEC3:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_3;
			break;
		case SGMII_FM2_DTSEC4:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_4;
			break;
		case XAUI_FM1:
		case XAUI_FM2:
			if (lane_prtcl == XAUI_FM1)
				serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1	|
						    FSL_CORENET_DEVDISR2_10GEC1;
			else
				serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2	|
						    FSL_CORENET_DEVDISR2_10GEC2;
			break;
		case AURORA:
			break;
		default:
			break;
		}

#endif
	}

#ifdef DEBUG
	puts("\n");
#endif

	for (idx = 0; idx < SRDS_MAX_BANK; idx++) {
		u32 rstctl;

		bank = idx;

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
		if (!IS_SVR_REV(get_svr(), 1, 0)) {
			/*
			 * Change bank init order to 0, 2, 1, so that the
			 * third bank's PLL is established before we
			 * start the second bank which shares the third
			 * bank's PLL.
			 */

			if (idx == 1)
				bank = FSL_SRDS_BANK_3;
			else if (idx == 2)
				bank = FSL_SRDS_BANK_2;
		}
#endif

		/* Skip disabled banks */
		if (!have_bank[bank])
			continue;

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
		if (!IS_SVR_REV(get_svr(), 1, 0)) {
			if (idx == 1) {
				p4080_erratum_serdes8(srds_regs, gur,
						      serdes8_devdisr,
						      serdes8_devdisr2, cfg);
			} else if (idx == 2) {
				enable_bank(gur, FSL_SRDS_BANK_2);
			}
		}
#endif

		/* reset banks for errata */
		setbits_be32(&srds_regs->bank[bank].rstctl, SRDS_RSTCTL_RST);

		/* wait for reset complete or 1-second timeout */
		end_tick = usec2ticks(1000000) + get_ticks();
		do {
			rstctl = in_be32(&srds_regs->bank[bank].rstctl);
			if (rstctl & SRDS_RSTCTL_RSTDONE)
				break;
		} while (end_tick > get_ticks());

		if (!(rstctl & SRDS_RSTCTL_RSTDONE)) {
			printf("SERDES: timeout resetting bank %d\n",
			       bank + 1);
			continue;
		}
	}
}
