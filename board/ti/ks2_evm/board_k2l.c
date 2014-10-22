/*
 * K2L EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/ti-aemif.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	= 122880000,
	[alt_core_clk]	= 100000000,
	[pa_clk]	= 122880000,
	[tetris_clk]	= 122880000,
	[ddr3_clk]	= 100000000,
	[pcie_clk]	= 100000000,
	[sgmii_clk]	= 156250000,
	[usb_clk]	= 100000000,
};

static struct pll_init_data core_pll_config[] = {
	CORE_PLL_799,
	CORE_PLL_1000,
	CORE_PLL_1198,
};

static struct pll_init_data tetris_pll_config[] = {
	TETRIS_PLL_799,
	TETRIS_PLL_1000,
	TETRIS_PLL_1198,
	TETRIS_PLL_1352,
	TETRIS_PLL_1401,
};

static struct pll_init_data pa_pll_config =
	PASS_PLL_983;

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	int speed;

	speed = get_max_dev_speed();
	init_pll(&core_pll_config[speed]);

	init_pll(&pa_pll_config);

	speed = get_max_arm_speed();
	init_pll(&tetris_pll_config[speed]);

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
static struct pll_init_data spl_pll_config[] = {
	CORE_PLL_799,
	TETRIS_PLL_491,
};

void spl_init_keystone_plls(void)
{
	init_plls(ARRAY_SIZE(spl_pll_config), spl_pll_config);
}
#endif
