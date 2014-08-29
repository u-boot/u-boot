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

static struct pll_init_data core_pll_config[] = {
	CORE_PLL_800,
	CORE_PLL_850,
	CORE_PLL_1000,
	CORE_PLL_1250,
	CORE_PLL_1350,
	CORE_PLL_1400,
	CORE_PLL_1500,
};


static struct pll_init_data pa_pll_config =
	PASS_PLL_1000;

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	int speed;

	speed = get_max_dev_speed();
	init_pll(&core_pll_config[speed]);

	init_pll(&pa_pll_config);

	return 0;
}
#endif
