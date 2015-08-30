/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ASM_ARCH_CLOCK_H
#define _ASM_ARCH_CLOCK_H

/* define pll mode */
#define RKCLK_PLL_MODE_SLOW		0
#define RKCLK_PLL_MODE_NORMAL		1

enum {
	ROCKCHIP_SYSCON_NOC,
	ROCKCHIP_SYSCON_GRF,
	ROCKCHIP_SYSCON_SGRF,
	ROCKCHIP_SYSCON_PMU,
};

/* Standard Rockchip clock numbers */
enum rk_clk_id {
	CLK_OSC,
	CLK_ARM,
	CLK_DDR,
	CLK_CODEC,
	CLK_GENERAL,
	CLK_NEW,

	CLK_COUNT,
};

static inline int rk_pll_id(enum rk_clk_id clk_id)
{
	return clk_id - 1;
}

/**
 * rockchip_get_cru() - get a pointer to the clock/reset unit registers
 *
 * @return pointer to registers, or -ve error on error
 */
void *rockchip_get_cru(void);

#endif
