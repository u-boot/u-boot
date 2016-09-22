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
	const struct uniphier_clk_gate_data *gate;
	unsigned int nr_gate;
	const struct uniphier_clk_rate_data *rate;
	unsigned int nr_rate;
};

#define UNIPHIER_CLK_FIXED_RATE(i, f)			\
	{						\
		.index = i,				\
		.reg = UNIPHIER_CLK_RATE_IS_FIXED,	\
		.rate = f,				\
	}

extern const struct uniphier_clk_soc_data uniphier_mio_clk_data;

#endif /* __CLK_UNIPHIER_H__ */
