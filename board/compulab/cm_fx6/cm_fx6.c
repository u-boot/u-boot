/*
 * Board functions for Compulab CM-FX6 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_esdhc.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include "common.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_NAND_MXS
static iomux_v3_cfg_t const nand_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void cm_fx6_setup_gpmi_nand(void)
{
	SETUP_IOMUX_PADS(nand_pads);
	/* Enable clock roots */
	enable_usdhc_clk(1, 3);
	enable_usdhc_clk(1, 4);

	setup_gpmi_io_clk(MXC_CCM_CS2CDR_ENFC_CLK_PODF(0xf) |
			  MXC_CCM_CS2CDR_ENFC_CLK_PRED(1)   |
			  MXC_CCM_CS2CDR_ENFC_CLK_SEL(0));
}
#else
static void cm_fx6_setup_gpmi_nand(void) {}
#endif

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
	{USDHC3_BASE_ADDR},
};

static enum mxc_clock usdhc_clk[3] = {
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_ESDHC3_CLK,
};

int board_mmc_init(bd_t *bis)
{
	int i;

	cm_fx6_set_usdhc_iomux();
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		usdhc_cfg[i].sdhc_clk = mxc_get_clock(usdhc_clk[i]);
		usdhc_cfg[i].max_bus_width = 4;
		fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		enable_usdhc_clk(1, i);
	}

	return 0;
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	cm_fx6_setup_gpmi_nand();

	return 0;
}

int checkboard(void)
{
	puts("Board: CM-FX6\n");
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;

	switch (gd->ram_size) {
	case 0x10000000: /* DDR_16BIT_256MB */
		gd->bd->bi_dram[0].size = 0x10000000;
		gd->bd->bi_dram[1].size = 0;
		break;
	case 0x20000000: /* DDR_32BIT_512MB */
		gd->bd->bi_dram[0].size = 0x20000000;
		gd->bd->bi_dram[1].size = 0;
		break;
	case 0x40000000:
		if (is_cpu_type(MXC_CPU_MX6SOLO)) { /* DDR_32BIT_1GB */
			gd->bd->bi_dram[0].size = 0x20000000;
			gd->bd->bi_dram[1].size = 0x20000000;
		} else { /* DDR_64BIT_1GB */
			gd->bd->bi_dram[0].size = 0x40000000;
			gd->bd->bi_dram[1].size = 0;
		}
		break;
	case 0x80000000: /* DDR_64BIT_2GB */
		gd->bd->bi_dram[0].size = 0x40000000;
		gd->bd->bi_dram[1].size = 0x40000000;
		break;
	case 0xEFF00000: /* DDR_64BIT_4GB */
		gd->bd->bi_dram[0].size = 0x70000000;
		gd->bd->bi_dram[1].size = 0x7FF00000;
		break;
	}
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	switch (gd->ram_size) {
	case 0x10000000:
	case 0x20000000:
	case 0x40000000:
	case 0x80000000:
		break;
	case 0xF0000000:
		gd->ram_size -= 0x100000;
		break;
	default:
		printf("ERROR: Unsupported DRAM size 0x%lx\n", gd->ram_size);
		return -1;
	}

	return 0;
}
