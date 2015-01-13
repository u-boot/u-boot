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
	init_pll(&ddr3_400);

	/* No SO-DIMM, 2GB discreet DDR */
	printf("DRAM: 2 GiB\n");
	ddr3_size = 2;

	/* Reset DDR3 PHY after PLL enabled */
	ddr3_reset_ddrphy();

	ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1600_2g);
	ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_2g);
}

/**
 * ddr3_get_size - return ddr3 size in GiB
 */
int ddr3_get_size(void)
{
	return ddr3_size;
}
