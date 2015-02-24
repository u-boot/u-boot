/*
 * K2L: Clock management APIs
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __ASM_ARCH_CLOCK_K2L_H
#define __ASM_ARCH_CLOCK_K2L_H

enum ext_clk_e {
	sys_clk,
	alt_core_clk,
	pa_clk,
	tetris_clk,
	ddr3_clk,
	pcie_clk,
	sgmii_clk,
	usb_clk,
	rp1_clk,
	ext_clk_count /* number of external clocks */
};

extern unsigned int external_clk[ext_clk_count];

#define CLK_LIST(CLK)\
	CLK(0, core_pll_clk)\
	CLK(1, pass_pll_clk)\
	CLK(2, tetris_pll_clk)\
	CLK(3, ddr3_pll_clk)\
	CLK(4, sys_clk0_clk)\
	CLK(5, sys_clk0_1_clk)\
	CLK(6, sys_clk0_2_clk)\
	CLK(7, sys_clk0_3_clk)\
	CLK(8, sys_clk0_4_clk)\
	CLK(9, sys_clk0_6_clk)\
	CLK(10, sys_clk0_8_clk)\
	CLK(11, sys_clk0_12_clk)\
	CLK(12, sys_clk0_24_clk)\
	CLK(13, sys_clk1_clk)\
	CLK(14, sys_clk1_3_clk)\
	CLK(15, sys_clk1_4_clk)\
	CLK(16, sys_clk1_6_clk)\
	CLK(17, sys_clk1_12_clk)\
	CLK(18, sys_clk2_clk)\
	CLK(19, sys_clk3_clk)\

#define PLLSET_CMD_LIST	"<pa|arm|ddr3>"

#define KS2_CLK1_6	sys_clk0_6_clk

/* PLL identifiers */
enum pll_type_e {
	CORE_PLL,
	PASS_PLL,
	TETRIS_PLL,
	DDR3_PLL,
};

enum {
	SPD800,
	SPD1000,
	SPD1200,
	SPD1350,
	SPD1400,
	SPD_RSV
};

#define CORE_PLL_799	{CORE_PLL, 13, 1, 2}
#define CORE_PLL_983	{CORE_PLL, 16, 1, 2}
#define CORE_PLL_1000	{CORE_PLL, 114, 7, 2}
#define CORE_PLL_1167	{CORE_PLL, 19, 1, 2}
#define CORE_PLL_1198	{CORE_PLL, 39, 2, 2}
#define CORE_PLL_1228	{CORE_PLL, 20, 1, 2}
#define PASS_PLL_1228	{PASS_PLL, 20, 1, 2}
#define PASS_PLL_983	{PASS_PLL, 16, 1, 2}
#define PASS_PLL_1050	{PASS_PLL, 205, 12, 2}
#define TETRIS_PLL_491	{TETRIS_PLL, 8, 1, 2}
#define TETRIS_PLL_737	{TETRIS_PLL, 12, 1, 2}
#define TETRIS_PLL_799	{TETRIS_PLL, 13, 1, 2}
#define TETRIS_PLL_983	{TETRIS_PLL, 16, 1, 2}
#define TETRIS_PLL_1000	{TETRIS_PLL, 114, 7, 2}
#define TETRIS_PLL_1167	{TETRIS_PLL, 19, 1, 2}
#define TETRIS_PLL_1198	{TETRIS_PLL, 39, 2, 2}
#define TETRIS_PLL_1228	{TETRIS_PLL, 20, 1, 2}
#define TETRIS_PLL_1352	{TETRIS_PLL, 22, 1, 2}
#define TETRIS_PLL_1401	{TETRIS_PLL, 114, 5, 2}
#define DDR3_PLL_200	{DDR3_PLL, 4, 1, 2}
#define DDR3_PLL_400	{DDR3_PLL, 16, 1, 4}
#define DDR3_PLL_800	{DDR3_PLL, 16, 1, 2}
#define DDR3_PLL_333	{DDR3_PLL, 20, 1, 6}

#endif
