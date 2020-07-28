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

struct k210_pll {
	struct clk clk;
	void __iomem *reg; /* Base PLL register */
	void __iomem *lock; /* Common PLL lock register */
	u8 shift; /* Offset of bits in lock register */
	u8 width; /* Width of lock bits to test against */
};

#define to_k210_pll(_clk) container_of(_clk, struct k210_pll, clk)

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

extern const struct clk_ops k210_pll_ops;

struct clk *k210_register_pll_struct(const char *name, const char *parent_name,
				     struct k210_pll *pll);
struct clk *k210_register_pll(const char *name, const char *parent_name,
			      void __iomem *reg, void __iomem *lock, u8 shift,
			      u8 width);

#endif /* K210_PLL_H */
