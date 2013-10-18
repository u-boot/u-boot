/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "../common/qixis.h"
#include "../common/vsc3316_3308.h"
#include "../common/idt8t49n222a_serdes_clk.h"
#include "b4860qds.h"
#include "b4860qds_qixis.h"
#include "b4860qds_crossbar_con.h"

#define CLK_MUX_SEL_MASK	0x4
#define ETH_PHY_CLK_OUT		0x4
#define PLL_NUM			2

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	char buf[64];
	u8 sw;
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *const freq[] = {"100", "125", "156.25", "161.13",
						"122.88", "122.88", "122.88"};
	int clock;

	printf("Board: %sQDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, ",
		QIXIS_READ(id), QIXIS_READ(arch));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw >= 0x8 && sw <= 0xE)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	printf("FPGA: v%d (%s), build %d",
		(int)QIXIS_READ(scver), qixis_read_tag(buf),
		(int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	/*
	 * Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES Reference Clocks: ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = (sw >> 5) & 7;
	printf("Bank1=%sMHz ", freq[clock]);
	sw = QIXIS_READ(brdcfg[4]);
	clock = (sw >> 6) & 3;
	printf("Bank2=%sMHz\n", freq[clock]);

	return 0;
}

int select_i2c_ch_pca(u8 ch)
{
	int ret;

	/* Selecting proper channel via PCA*/
	ret = i2c_write(I2C_MUX_PCA_ADDR, 0x0, 1, &ch, 1);
	if (ret) {
		printf("PCA: failed to select proper channel.\n");
		return ret;
	}

	return 0;
}

int configure_vsc3316_3308(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned int num_vsc16_con, num_vsc08_con;
	u32 serdes1_prtcl, serdes2_prtcl;
	int ret;

	serdes1_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	if (!serdes1_prtcl) {
		printf("SERDES1 is not enabled\n");
		return 0;
	}
	serdes1_prtcl >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	debug("Using SERDES1 Protocol: 0x%x:\n", serdes1_prtcl);

	serdes2_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	if (!serdes2_prtcl) {
		printf("SERDES2 is not enabled\n");
		return 0;
	}
	serdes2_prtcl >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	debug("Using SERDES2 Protocol: 0x%x:\n", serdes2_prtcl);

	switch (serdes1_prtcl) {
	case 0x2a:
	case 0x2C:
	case 0x2D:
	case 0x2E:
			/*
			 * Configuration:
			 * SERDES: 1
			 * Lanes: A,B: SGMII
			 * Lanes: C,D,E,F,G,H: CPRI
			 */
		debug("Configuring crossbar to use onboard SGMII PHYs:"
				"srds_prctl:%x\n", serdes1_prtcl);
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_4sfp_sgmii_12_56,
					num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_4sfp_sgmii_12_56,
					num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;

#ifdef CONFIG_PPC_B4420
	case 0x18:
			/*
			 * Configuration:
			 * SERDES: 1
			 * Lanes: A,B,C,D: SGMII
			 * Lanes: E,F,G,H: CPRI
			 */
		debug("Configuring crossbar to use onboard SGMII PHYs:"
				"srds_prctl:%x\n", serdes1_prtcl);
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_sgmii_lane_cd, num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_sgmii_lane_cd, num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;
#endif

	case 0x3E:
	case 0x0D:
	case 0x0E:
	case 0x12:
		num_vsc16_con = NUM_CON_VSC3316;
		/* Configure VSC3316 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3316);
		if (!ret) {
			ret = vsc3316_config(VSC3316_TX_ADDRESS,
					vsc16_tx_sfp, num_vsc16_con);
			if (ret)
				return ret;
			ret = vsc3316_config(VSC3316_RX_ADDRESS,
					vsc16_rx_sfp, num_vsc16_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;
	default:
		printf("WARNING:VSC crossbars programming not supported for:%x"
					" SerDes1 Protocol.\n", serdes1_prtcl);
		return -1;
	}

	switch (serdes2_prtcl) {
	case 0x9E:
	case 0x9A:
	case 0x98:
	case 0xb2:
	case 0x49:
	case 0x4E:
	case 0x8D:
	case 0x7A:
		num_vsc08_con = NUM_CON_VSC3308;
		/* Configure VSC3308 crossbar switch */
		ret = select_i2c_ch_pca(I2C_CH_VSC3308);
		if (!ret) {
			ret = vsc3308_config(VSC3308_TX_ADDRESS,
					vsc08_tx_amc, num_vsc08_con);
			if (ret)
				return ret;
			ret = vsc3308_config(VSC3308_RX_ADDRESS,
					vsc08_rx_amc, num_vsc08_con);
			if (ret)
				return ret;
		} else {
			return ret;
		}
		break;
	default:
		printf("WARNING:VSC crossbars programming not supported for: %x"
					" SerDes2 Protocol.\n", serdes2_prtcl);
		return -1;
	}

	return 0;
}

int config_serdes1_refclks(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 serdes1_prtcl, lane;
	unsigned int flag_sgmii_prtcl = 0;
	int ret, i;

	serdes1_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	if (!serdes1_prtcl) {
		printf("SERDES1 is not enabled\n");
		return -1;
	}
	serdes1_prtcl >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	debug("Using SERDES1 Protocol: 0x%x:\n", serdes1_prtcl);

	/* Clear SRDS_RSTCTL_RST bit for both PLLs before changing refclks
	 */
	for (i = 0; i < PLL_NUM; i++)
		clrbits_be32(&srds_regs->bank[i].rstctl, SRDS_RSTCTL_RST);
	/* Reconfigure IDT idt8t49n222a device for CPRI to work
	 * For this SerDes1's Refclk1 and refclk2 need to be set
	 * to 122.88MHz
	 */
	switch (serdes1_prtcl) {
	case 0x2A:
	case 0x2C:
	case 0x2D:
	case 0x2E:
		debug("Configuring idt8t49n222a for CPRI SerDes clks:"
			" for srds_prctl:%x\n", serdes1_prtcl);
		ret = select_i2c_ch_pca(I2C_CH_IDT);
		if (!ret) {
			ret = set_serdes_refclk(IDT_SERDES1_ADDRESS, 1,
					SERDES_REFCLK_122_88,
					SERDES_REFCLK_122_88, 0);
			if (ret) {
				printf("IDT8T49N222A configuration failed.\n");
				return ret;
			} else
				printf("IDT8T49N222A configured.\n");
		} else {
			return ret;
		}
		select_i2c_ch_pca(I2C_CH_DEFAULT);

		/* Change SerDes1's Refclk1 to 125MHz for on board
		 * SGMIIs to work
		 */
		for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
			enum srds_prtcl lane_prtcl = serdes_get_prtcl
						(0, serdes1_prtcl, lane);
			switch (lane_prtcl) {
			case SGMII_FM1_DTSEC1:
			case SGMII_FM1_DTSEC2:
			case SGMII_FM1_DTSEC3:
			case SGMII_FM1_DTSEC4:
			case SGMII_FM1_DTSEC5:
			case SGMII_FM1_DTSEC6:
				flag_sgmii_prtcl++;
				break;
			default:
				break;
			}
		}

		if (flag_sgmii_prtcl)
			QIXIS_WRITE(brdcfg[4], QIXIS_SRDS1CLK_125);

		/* Steps For SerDes PLLs reset and reconfiguration after
		 * changing SerDes's refclks
		 */
		for (i = 0; i < PLL_NUM; i++) {
			debug("For PLL%d reset and reconfiguration after"
			       " changing refclks\n", i+1);
			clrbits_be32(&srds_regs->bank[i].rstctl,
					SRDS_RSTCTL_SDRST_B);
			udelay(10);
			clrbits_be32(&srds_regs->bank[i].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B));
			udelay(10);
			setbits_be32(&srds_regs->bank[i].rstctl,
					SRDS_RSTCTL_RST);
			setbits_be32(&srds_regs->bank[i].rstctl,
				(SRDS_RSTCTL_SDEN | SRDS_RSTCTL_PLLRST_B
				| SRDS_RSTCTL_SDRST_B));
		}
		break;
	default:
		printf("WARNING:IDT8T49N222A configuration not"
			" supported for:%x SerDes1 Protocol.\n",
			serdes1_prtcl);
		return -1;
	}

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash + promjet */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	set_liodns();
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_portals();
#endif
	/* SerDes1 refclks need to be set again, as default clks
	 * are not suitable for CPRI and onboard SGMIIs to work
	 * simultaneously.
	 * This function will set SerDes1's Refclk1 and refclk2
	 * as per SerDes1 protocols
	 */
	if (config_serdes1_refclks())
		printf("SerDes1 Refclks couldn't set properly.\n");
	else
		printf("SerDes1 Refclks have been set.\n");

	/* Configure VSC3316 and VSC3308 crossbar switches */
	if (configure_vsc3316_3308())
		printf("VSC:failed to configure VSC3316/3308.\n");
	else
		printf("VSC:VSC3316/3308 successfully configured.\n");

	select_i2c_ch_pca(I2C_CH_DEFAULT);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((sysclk_conf & 0x0C) >> 2) {
	case QIXIS_CLK_100:
		return 100000000;
	case QIXIS_CLK_125:
		return 125000000;
	case QIXIS_CLK_133:
		return 133333333;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch (ddrclk_conf & 0x03) {
	case QIXIS_CLK_100:
		return 100000000;
	case QIXIS_CLK_125:
		return 125000000;
	case QIXIS_CLK_133:
		return 133333333;
	}
	return 66666666;
}

static int serdes_refclock(u8 sw, u8 sdclk)
{
	unsigned int clock;
	int ret = -1;
	u8 brdcfg4;

	if (sdclk == 1) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		if ((brdcfg4 & CLK_MUX_SEL_MASK) == ETH_PHY_CLK_OUT)
			return SRDS_PLLCR0_RFCK_SEL_125;
		else
			clock = (sw >> 5) & 7;
	} else
		clock = (sw >> 6) & 3;

	switch (clock) {
	case 0:
		ret = SRDS_PLLCR0_RFCK_SEL_100;
		break;
	case 1:
		ret = SRDS_PLLCR0_RFCK_SEL_125;
		break;
	case 2:
		ret = SRDS_PLLCR0_RFCK_SEL_156_25;
		break;
	case 3:
		ret = SRDS_PLLCR0_RFCK_SEL_161_13;
		break;
	case 4:
	case 5:
	case 6:
		ret = SRDS_PLLCR0_RFCK_SEL_122_88;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

#define NUM_SRDS_BANKS	2

int misc_init_r(void)
{
	u8 sw;
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[NUM_SRDS_BANKS];
	unsigned int i;
	int clock;

	sw = QIXIS_READ(brdcfg[2]);
	clock = serdes_refclock(sw, 1);
	if (clock >= 0)
		actual[0] = clock;
	else
		printf("Warning: SDREFCLK1 switch setting is unsupported\n");

	sw = QIXIS_READ(brdcfg[4]);
	clock = serdes_refclock(sw, 2);
	if (clock >= 0)
		actual[1] = clock;
	else
		printf("Warning: SDREFCLK2 switch setting unsupported\n");

	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 pllcr0 = srds_regs->bank[i].pllcr0;
		u32 expected = pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES bank %u expects reference clock"
			       " %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

	return 0;
}

void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);

#ifdef CONFIG_HAS_FSL_DR_USB
	fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif
}

/*
 * Dump board switch settings.
 * The bits that cannot be read/sampled via some FPGA or some
 * registers, they will be displayed as
 * underscore in binary format. mask[] has those bits.
 * Some bits are calculated differently than the actual switches
 * if booting with overriding by FPGA.
 */
void qixis_dump_switch(void)
{
	int i;
	u8 sw[5];

	/*
	 * Any bit with 1 means that bit cannot be reverse engineered.
	 * It will be displayed as _ in binary format.
	 */
	static const u8 mask[] = {0x07, 0, 0, 0xff, 0};
	char buf[10];
	u8 brdcfg[16], dutcfg[16];

	for (i = 0; i < 16; i++) {
		brdcfg[i] = qixis_read(offsetof(struct qixis, brdcfg[0]) + i);
		dutcfg[i] = qixis_read(offsetof(struct qixis, dutcfg[0]) + i);
	}

	sw[0] = ((brdcfg[0] & 0x0f) << 4)	| \
		(brdcfg[9] & 0x08);
	sw[1] = ((dutcfg[1] & 0x01) << 7)	| \
		((dutcfg[2] & 0x07) << 4)       | \
		((dutcfg[6] & 0x10) >> 1)       | \
		((dutcfg[6] & 0x80) >> 5)       | \
		((dutcfg[1] & 0x40) >> 5)       | \
		(dutcfg[6] & 0x01);
	sw[2] = dutcfg[0];
	sw[3] = 0;
	sw[4] = ((brdcfg[1] & 0x30) << 2)	| \
		((brdcfg[1] & 0xc0) >> 2)	| \
		(brdcfg[1] & 0x0f);

	puts("DIP switch settings:\n");
	for (i = 0; i < 5; i++) {
		printf("SW%d         = 0b%s (0x%02x)\n",
			i + 1, byte_to_binary_mask(sw[i], mask[i], buf), sw[i]);
	}
}
