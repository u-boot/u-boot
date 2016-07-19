/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CLK_UNIPHIER_H__
#define __CLK_UNIPHIER_H__

#include <linux/kernel.h>

struct uniphier_clk_gate_data {
	int index;
	unsigned int reg;
	u32 mask;
	u32 data;
};

struct uniphier_clk_rate_data {
	int index;
	unsigned int reg;
#define UNIPHIER_CLK_RATE_IS_FIXED		UINT_MAX
	u32 mask;
	u32 data;
	unsigned long rate;
};

struct uniphier_clk_soc_data {
	struct uniphier_clk_gate_data *gate;
	unsigned int nr_gate;
	struct uniphier_clk_rate_data *rate;
	unsigned int nr_rate;
};

#define UNIPHIER_CLK_FIXED_RATE(i, f)			\
	{						\
		.index = i,				\
		.reg = UNIPHIER_CLK_RATE_IS_FIXED,	\
		.rate = f,				\
	}

/**
 * struct uniphier_clk_priv - private data for UniPhier clock driver
 *
 * @base: base address of the clock provider
 * @socdata: SoC specific data
 */
struct uniphier_clk_priv {
	void __iomem *base;
	struct uniphier_clk_soc_data *socdata;
};

extern const struct clk_ops uniphier_clk_ops;
int uniphier_clk_probe(struct udevice *dev);

#endif /* __CLK_UNIPHIER_H__ */
