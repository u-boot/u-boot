/*
 * K2G EVM : Board initialization
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clock.h>

#define SYS_CLK		24000000

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	=	SYS_CLK,
	[pa_clk]	=	SYS_CLK,
	[tetris_clk]	=	SYS_CLK,
	[ddr3a_clk]	=	SYS_CLK,
	[uart_clk]	=	SYS_CLK,
};

static struct pll_init_data main_pll_config = {MAIN_PLL, 100, 1, 4};
static struct pll_init_data tetris_pll_config = {TETRIS_PLL, 100, 1, 4};
static struct pll_init_data uart_pll_config = {UART_PLL, 64, 1, 4};
static struct pll_init_data nss_pll_config = {NSS_PLL, 250, 3, 2};
static struct pll_init_data ddr3_pll_config = {DDR3A_PLL, 250, 3, 10};

struct pll_init_data *get_pll_init_data(int pll)
{
	struct pll_init_data *data = NULL;

	switch (pll) {
	case MAIN_PLL:
		data = &main_pll_config;
		break;
	case TETRIS_PLL:
		data = &tetris_pll_config;
		break;
	case NSS_PLL:
		data = &nss_pll_config;
		break;
	case UART_PLL:
		data = &uart_pll_config;
		break;
	case DDR3_PLL:
		data = &ddr3_pll_config;
		break;
	default:
		data = NULL;
	}

	return data;
}

s16 divn_val[16] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	init_plls();

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void spl_init_keystone_plls(void)
{
	init_plls();
}
#endif
