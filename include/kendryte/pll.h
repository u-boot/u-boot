/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */
#ifndef K210_PLL_H
#define K210_PLL_H

#include <clk.h>
#include <test/export.h>
#include <asm/io.h>

#define K210_PLL_CLKR GENMASK(3, 0)
#define K210_PLL_CLKF GENMASK(9, 4)
#define K210_PLL_CLKOD GENMASK(13, 10) /* Output Divider */
#define K210_PLL_BWADJ GENMASK(19, 14) /* BandWidth Adjust */
#define K210_PLL_RESET BIT(20)
#define K210_PLL_PWRD BIT(21) /* PoWeReD */
#define K210_PLL_INTFB BIT(22) /* Internal FeedBack */
#define K210_PLL_BYPASS BIT(23)
#define K210_PLL_TEST BIT(24)
#define K210_PLL_EN BIT(25)
#define K210_PLL_TEST_EN BIT(26)

#define K210_PLL_LOCK 0
#define K210_PLL_CLEAR_SLIP 2
#define K210_PLL_TEST_OUT 3

struct k210_pll_config {
	u8 r;
	u8 f;
	u8 od;
};

#ifdef CONFIG_UNIT_TEST
TEST_STATIC int k210_pll_calc_config(u32 rate, u32 rate_in,
				     struct k210_pll_config *best);

#ifndef nop
#define nop()
#endif

#endif

/**
 * struct k210_clk_priv - K210 clock driver private data
 * @base: The base address of the sysctl device
 * @in0: The "in0" external oscillator
 */
struct k210_clk_priv {
	void __iomem *base;
	struct clk in0;
};

ulong k210_pll_set_rate(struct k210_clk_priv *priv, int id, ulong rate, ulong rate_in);
ulong k210_pll_get_rate(struct k210_clk_priv *priv, int id, ulong rate_in);
int k210_pll_enable(struct k210_clk_priv *priv, int id);
int k210_pll_disable(struct k210_clk_priv *priv, int id);
#endif /* K210_PLL_H */
