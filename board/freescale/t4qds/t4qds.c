/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
#include "t4qds.h"
#include "t4240qds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	u8 sw;
	struct cpu_type *cpu = gd->cpu;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	unsigned int i;

	printf("Board: %sQDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		QIXIS_READ(id), QIXIS_READ(arch), QIXIS_READ(scver));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("Promjet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	/* Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		u32 rcw = in_be32(&gur->rcwsr[i]);

		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

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
	for (i = 0; i < MAX_SERDES; i++) {
		static const char *freq[] = {
			"100", "125", "156.25", "161.1328125"};
		unsigned int clock = (sw >> (2 * i)) & 3;

		printf("SERDES%u=%sMHz ", i+1, freq[clock]);
	}
	puts("\n");

	return 0;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

/* Configure Crossbar switches for Front-Side SerDes Ports */
int config_frontside_crossbar_vsc3316(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s1, srds_prtcl_s2;
	int ret;

	ret = select_i2c_ch_pca9547(I2C_MUX_CH_VSC3316_FS);
	if (ret)
		return ret;

	srds_prtcl_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_prtcl_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
	if (srds_prtcl_s1) {
		ret = vsc3316_config(VSC3316_FSM_TX_ADDR, vsc3316_fsm1_tx, 8);
		if (ret)
			return ret;
		ret = vsc3316_config(VSC3316_FSM_RX_ADDR, vsc3316_fsm1_rx, 8);
		if (ret)
			return ret;
	}

	srds_prtcl_s2 = in_be32(&gur->rcwsr[4]) &
				FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
	srds_prtcl_s2 >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
	if (srds_prtcl_s2) {
		ret = vsc3316_config(VSC3316_FSM_TX_ADDR, vsc3316_fsm2_tx, 8);
		if (ret)
			return ret;
		ret = vsc3316_config(VSC3316_FSM_RX_ADDR, vsc3316_fsm2_rx, 8);
		if (ret)
			return ret;
	}

	return 0;
}

int config_backside_crossbar_mux(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_prtcl_s3, srds_prtcl_s4;
	u8 brdcfg;

	srds_prtcl_s3 = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS3_PRTCL;
	srds_prtcl_s3 >>= FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT;
	switch (srds_prtcl_s3) {
	case 0:
		/* SerDes3 is not enabled */
		break;
	case 2:
	case 9:
	case 10:
		/* SD3(0:7) => SLOT5(0:7) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD3MX_MASK;
		brdcfg |= BRDCFG12_SD3MX_SLOT5;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	case 4:
	case 6:
	case 8:
	case 12:
	case 14:
	case 16:
	case 17:
	case 19:
	case 20:
		/* SD3(4:7) => SLOT6(0:3) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD3MX_MASK;
		brdcfg |= BRDCFG12_SD3MX_SLOT6;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	default:
		printf("WARNING: unsupported for SerDes3 Protocol %d\n",
				srds_prtcl_s3);
		return -1;
	}

	srds_prtcl_s4 = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS4_PRTCL;
	srds_prtcl_s4 >>= FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT;
	switch (srds_prtcl_s4) {
	case 0:
		/* SerDes4 is not enabled */
		break;
	case 2:
		/* 10b, SD4(0:7) => SLOT7(0:7) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD4MX_MASK;
		brdcfg |= BRDCFG12_SD4MX_SLOT7;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	case 4:
	case 6:
	case 8:
		/* x1b, SD4(4:7) => SLOT8(0:3) */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD4MX_MASK;
		brdcfg |= BRDCFG12_SD4MX_SLOT8;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
		/* 00b, SD4(4:5) => AURORA, SD4(6:7) => SATA */
		brdcfg = QIXIS_READ(brdcfg[12]);
		brdcfg &= ~BRDCFG12_SD4MX_MASK;
		brdcfg |= BRDCFG12_SD4MX_AURO_SATA;
		QIXIS_WRITE(brdcfg[12], brdcfg);
		break;
	default:
		printf("WARNING: unsupported for SerDes4 Protocol %d\n",
				srds_prtcl_s4);
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

	/* Disable remote I2C connectoin */
	QIXIS_WRITE(brdcfg[5], BRDCFG5_RESET);

	/* Configure board SERDES ports crossbar */
	config_frontside_crossbar_vsc3316();
	config_backside_crossbar_mux();
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0F) {
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 66666666;
}

static const char *serdes_clock_to_string(u32 clock)
{
	switch (clock) {
	case SRDS_PLLCR0_RFCK_SEL_100:
		return "100";
	case SRDS_PLLCR0_RFCK_SEL_125:
		return "125";
	case SRDS_PLLCR0_RFCK_SEL_156_25:
		return "156.25";
	case SRDS_PLLCR0_RFCK_SEL_161_13:
		return "161.1328125";
	default:
		return "???";
	}
}

int misc_init_r(void)
{
	u8 sw;
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[MAX_SERDES];
	unsigned int i;

	sw = QIXIS_READ(brdcfg[2]);
	for (i = 0; i < MAX_SERDES; i++) {
		unsigned int clock = (sw >> (2 * i)) & 3;
		switch (clock) {
		case 0:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_100;
			break;
		case 1:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		case 2:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		case 3:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_161_13;
			break;
		}
	}

	for (i = 0; i < MAX_SERDES; i++) {
		u32 pllcr0 = srds_regs->bank[i].pllcr0;
		u32 expected = pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES%u expects reference clock"
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
	fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif
}
