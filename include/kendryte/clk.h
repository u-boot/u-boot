/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */

#ifndef K210_CLK_H
#define K210_CLK_H

#define LOG_CATEGORY UCLASS_CLK
#include <linux/types.h>
#include <linux/clk-provider.h>

static inline struct clk *k210_clk_gate(const char *name,
					const char *parent_name,
					void __iomem *reg, u8 bit_idx)
{
	return clk_register_gate(NULL, name, parent_name, 0, reg, bit_idx, 0,
				 NULL);
}

static inline struct clk *k210_clk_half(const char *name,
					const char *parent_name)
{
	return clk_register_fixed_factor(NULL, name, parent_name, 0, 1, 2);
}

static inline struct clk *k210_clk_div(const char *name,
				       const char *parent_name,
				       void __iomem *reg, u8 shift, u8 width)
{
	return clk_register_divider(NULL, name, parent_name, 0, reg, shift,
				    width, 0);
}

#endif /* K210_CLK_H */
