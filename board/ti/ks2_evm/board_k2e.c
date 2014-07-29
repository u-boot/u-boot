/*
 * K2E EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	= 100000000,
	[alt_core_clk]	= 100000000,
	[pa_clk]	= 100000000,
	[ddr3_clk]	= 100000000,
	[mcm_clk]	= 312500000,
	[pcie_clk]	= 100000000,
	[sgmii_clk]	= 156250000,
	[xgmii_clk]	= 156250000,
	[usb_clk]	= 100000000,
};

static struct pll_init_data pll_config[] = {
	CORE_PLL_1200,
	PASS_PLL_1000,
};

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	init_plls(ARRAY_SIZE(pll_config), pll_config);
	return 0;
}
#endif
