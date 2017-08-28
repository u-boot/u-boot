/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CLK_UNIPHIER_H__
#define __CLK_UNIPHIER_H__

#include <linux/kernel.h>

#define UNIPHIER_CLK_MAX_NR_MUXS	8

struct uniphier_clk_gate_data {
	unsigned int id;
	unsigned int reg;
	unsigned int bit;
};

struct uniphier_clk_mux_data {
	unsigned int id;
	unsigned int nr_muxs;
	unsigned int reg;
	unsigned int masks[UNIPHIER_CLK_MAX_NR_MUXS];
	unsigned int vals[UNIPHIER_CLK_MAX_NR_MUXS];
	unsigned long rates[UNIPHIER_CLK_MAX_NR_MUXS];
};

struct uniphier_clk_data {
	const struct uniphier_clk_gate_data *gate;
	const struct uniphier_clk_mux_data *mux;
};

#define UNIPHIER_CLK_ID_END		(unsigned int)(-1)

#define UNIPHIER_CLK_END				\
	{ .id = UNIPHIER_CLK_ID_END }

#define UNIPHIER_CLK_GATE(_id, _reg, _bit)		\
	{						\
		.id = (_id),				\
		.reg = (_reg),				\
		.bit = (_bit),				\
	}

#define UNIPHIER_CLK_FIXED_RATE(_id, _rate)		\
	{						\
		.id = (_id),				\
		.rates = {(_reg),},			\
	}

extern const struct uniphier_clk_data uniphier_pxs2_sys_clk_data;
extern const struct uniphier_clk_data uniphier_ld20_sys_clk_data;
extern const struct uniphier_clk_data uniphier_mio_clk_data;

#endif /* __CLK_UNIPHIER_H__ */
