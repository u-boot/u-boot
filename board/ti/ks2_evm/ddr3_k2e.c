/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include "ddr3_cfg.h"
#include <asm/arch/ddr3.h>

static int ddr3_size;
static struct pll_init_data ddr3_400 = DDR3_PLL_400;

void ddr3_init(void)
{
	char dimm_name[32];

	if (~(readl(KS2_PLL_CNTRL_BASE + KS2_RSTCTRL_RSTYPE) & 0x1))
		init_pll(&ddr3_400);

	ddr3_get_dimm_params(dimm_name);

	printf("Detected SO-DIMM [%s]\n", dimm_name);

	/* Reset DDR3 PHY after PLL enabled */
	ddr3_reset_ddrphy();

	if (!strcmp(dimm_name, "18KSF1G72HZ-1G6E2 ")) {
		/* 8G SO-DIMM */
		ddr3_size = 8;
		printf("DRAM: 8 GiB\n");
		ddr3phy_1600_8g.zq0cr1 |= 0x10000;
		ddr3phy_1600_8g.zq1cr1 |= 0x10000;
		ddr3phy_1600_8g.zq2cr1 |= 0x10000;
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1600_8g);
		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_8g);
	} else if (!strcmp(dimm_name, "18KSF51272HZ-1G6K2")) {
		/* 4G SO-DIMM */
		ddr3_size = 4;
		printf("DRAM: 4 GiB\n");
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1600_4g);
		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_4g);
	}
}

/**
 * ddr3_get_size - return ddr3 size in GiB
 */
int ddr3_get_size(void)
{
	return ddr3_size;
}
