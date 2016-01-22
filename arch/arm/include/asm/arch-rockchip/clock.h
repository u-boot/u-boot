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
 * clk_get_divisor() - Calculate the required clock divisior
 *
 * Given an input rate and a required output_rate, calculate the Rockchip
 * divisor needed to achieve this.
 *
 * @input_rate:		Input clock rate in Hz
 * @output_rate:	Output clock rate in Hz
 * @return divisor register value to use
 */
static inline u32 clk_get_divisor(ulong input_rate, uint output_rate)
{
	uint clk_div;

	clk_div = input_rate / output_rate;
	clk_div = (clk_div + 1) & 0xfffe;

	return clk_div;
}

/**
 * rockchip_get_cru() - get a pointer to the clock/reset unit registers
 *
 * @return pointer to registers, or -ve error on error
 */
void *rockchip_get_cru(void);

/**
 * rkclk_get_clk() - get a pointer to a given clock
 *
 * This is an internal function - use outside the clock subsystem indicates
 * that work is needed!
 *
 * @clk_id:	Clock requested
 * @devp:	Returns a pointer to that clock
 * @return 0 if OK, -ve on error
 */
int rkclk_get_clk(enum rk_clk_id clk_id, struct udevice **devp);

struct rk3288_cru;
struct rk3288_grf;

void rkclk_configure_cpu(struct rk3288_cru *cru, struct rk3288_grf *grf);

#endif
