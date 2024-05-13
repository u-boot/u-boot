// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * This file includes utility functions to register clocks to common
 * clock framework for Samsung platforms.
 */

#include <dm.h>
#include "clk.h"

static void samsung_clk_register_mux(void __iomem *base, unsigned int cmu_id,
				     const struct samsung_mux_clock *clk_list,
				     unsigned int nr_clk)
{
	unsigned int cnt;

	for (cnt = 0; cnt < nr_clk; cnt++) {
		struct clk *clk;
		const struct samsung_mux_clock *m;
		unsigned long clk_id;

		m = &clk_list[cnt];
		clk = clk_register_mux(NULL, m->name, m->parent_names,
			m->num_parents, m->flags, base + m->offset, m->shift,
			m->width, m->mux_flags);
		clk_id = SAMSUNG_TO_CLK_ID(cmu_id, m->id);
		clk_dm(clk_id, clk);
	}
}

static void samsung_clk_register_div(void __iomem *base, unsigned int cmu_id,
				     const struct samsung_div_clock *clk_list,
				     unsigned int nr_clk)
{
	unsigned int cnt;

	for (cnt = 0; cnt < nr_clk; cnt++) {
		struct clk *clk;
		const struct samsung_div_clock *d;
		unsigned long clk_id;

		d = &clk_list[cnt];
		clk = clk_register_divider(NULL, d->name, d->parent_name,
			d->flags, base + d->offset, d->shift,
			d->width, d->div_flags);
		clk_id = SAMSUNG_TO_CLK_ID(cmu_id, d->id);
		clk_dm(clk_id, clk);
	}
}

static void samsung_clk_register_gate(void __iomem *base, unsigned int cmu_id,
				      const struct samsung_gate_clock *clk_list,
				      unsigned int nr_clk)
{
	unsigned int cnt;

	for (cnt = 0; cnt < nr_clk; cnt++) {
		struct clk *clk;
		const struct samsung_gate_clock *g;
		unsigned long clk_id;

		g = &clk_list[cnt];
		clk = clk_register_gate(NULL, g->name, g->parent_name,
			g->flags, base + g->offset, g->bit_idx,
			g->gate_flags, NULL);
		clk_id = SAMSUNG_TO_CLK_ID(cmu_id, g->id);
		clk_dm(clk_id, clk);
	}
}

typedef void (*samsung_clk_register_fn)(void __iomem *base, unsigned int cmu_id,
					const void *clk_list,
					unsigned int nr_clk);

static const samsung_clk_register_fn samsung_clk_register_fns[] = {
	[S_CLK_MUX]	= (samsung_clk_register_fn)samsung_clk_register_mux,
	[S_CLK_DIV]	= (samsung_clk_register_fn)samsung_clk_register_div,
	[S_CLK_GATE]	= (samsung_clk_register_fn)samsung_clk_register_gate,
	[S_CLK_PLL]	= (samsung_clk_register_fn)samsung_clk_register_pll,
};

/**
 * samsung_cmu_register_clocks() - Register provided clock groups
 * @base: Base address of CMU registers
 * @cmu_id: CMU index number
 * @clk_groups: list of clock groups
 * @nr_groups: count of clock groups in @clk_groups
 *
 * Having the array of clock groups @clk_groups makes it possible to keep a
 * correct clocks registration order.
 */
static void samsung_cmu_register_clocks(void __iomem *base, unsigned int cmu_id,
				const struct samsung_clk_group *clk_groups,
				unsigned int nr_groups)
{
	unsigned int i;

	for (i = 0; i < nr_groups; i++) {
		const struct samsung_clk_group *g = &clk_groups[i];

		samsung_clk_register_fns[g->type](base, cmu_id,
						  g->clk_list, g->nr_clk);
	}
}

/**
 * samsung_cmu_register_one - Register all CMU clocks
 * @dev: CMU device
 * @cmu_id: CMU index number
 * @clk_groups: list of CMU clock groups
 * @nr_groups: count of CMU clock groups in @clk_groups
 *
 * Return: 0 on success or negative value on error.
 */
int samsung_cmu_register_one(struct udevice *dev, unsigned int cmu_id,
			     const struct samsung_clk_group *clk_groups,
			     unsigned int nr_groups)
{
	void __iomem *base;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	samsung_cmu_register_clocks(base, cmu_id, clk_groups, nr_groups);

	return 0;
}
