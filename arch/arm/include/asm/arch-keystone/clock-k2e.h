/*
 * K2E: Clock management APIs
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __ASM_ARCH_CLOCK_K2E_H
#define __ASM_ARCH_CLOCK_K2E_H

enum ext_clk_e {
	sys_clk,
	alt_core_clk,
	pa_clk,
	ddr3_clk,
	mcm_clk,
	pcie_clk,
	sgmii_clk,
	xgmii_clk,
	usb_clk,
	ext_clk_count /* number of external clocks */
};

extern unsigned int external_clk[ext_clk_count];

#define CLK_LIST(CLK)\
	CLK(0, core_pll_clk)\
	CLK(1, pass_pll_clk)\
	CLK(2, ddr3_pll_clk)\
	CLK(3, sys_clk0_clk)\
	CLK(4, sys_clk0_1_clk)\
	CLK(5, sys_clk0_2_clk)\
	CLK(6, sys_clk0_3_clk)\
	CLK(7, sys_clk0_4_clk)\
	CLK(8, sys_clk0_6_clk)\
	CLK(9, sys_clk0_8_clk)\
	CLK(10, sys_clk0_12_clk)\
	CLK(11, sys_clk0_24_clk)\
	CLK(12, sys_clk1_clk)\
	CLK(13, sys_clk1_3_clk)\
	CLK(14, sys_clk1_4_clk)\
	CLK(15, sys_clk1_6_clk)\
	CLK(16, sys_clk1_12_clk)\
	CLK(17, sys_clk2_clk)\
	CLK(18, sys_clk3_clk)

#define PLLSET_CMD_LIST	"<pa|ddr3>"

#define KS2_CLK1_6	sys_clk0_6_clk

/* PLL identifiers */
enum pll_type_e {
	CORE_PLL,
	PASS_PLL,
	DDR3_PLL,
};

enum {
	SPD800,
	SPD850,
	SPD1000,
	SPD1250,
	SPD1350,
	SPD1400,
	SPD1500,
	SPD_RSV
};

#define CORE_PLL_800	{CORE_PLL, 16, 1, 2}
#define CORE_PLL_850	{CORE_PLL, 17, 1, 2}
#define CORE_PLL_1000	{CORE_PLL, 20, 1, 2}
#define CORE_PLL_1200	{CORE_PLL, 24, 1, 2}
#define PASS_PLL_1000	{PASS_PLL, 20, 1, 2}
#define CORE_PLL_1250	{CORE_PLL, 25, 1, 2}
#define CORE_PLL_1350	{CORE_PLL, 27, 1, 2}
#define CORE_PLL_1400	{CORE_PLL, 28, 1, 2}
#define CORE_PLL_1500	{CORE_PLL, 30, 1, 2}
#define DDR3_PLL_200	{DDR3_PLL, 4,  1, 2}
#define DDR3_PLL_400	{DDR3_PLL, 16, 1, 4}
#define DDR3_PLL_800	{DDR3_PLL, 16, 1, 2}
#define DDR3_PLL_333	{DDR3_PLL, 20, 1, 6}

#endif
