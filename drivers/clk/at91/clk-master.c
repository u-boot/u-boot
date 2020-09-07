// SPDX-License-Identifier: GPL-2.0+
/*
 * Master clock support for AT91 architectures.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-master.c from Linux.
 */

#include <asm/processor.h>
#include <clk-uclass.h>
#include <common.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_MASTER		"at91-master-clk"

#define MASTER_PRES_MASK	0x7
#define MASTER_PRES_MAX		MASTER_PRES_MASK
#define MASTER_DIV_SHIFT	8
#define MASTER_DIV_MASK		0x3

struct clk_master {
	void __iomem *base;
	const struct clk_master_layout *layout;
	const struct clk_master_characteristics *characteristics;
	const u32 *mux_table;
	const u32 *clk_mux_table;
	u32 num_parents;
	struct clk clk;
	u8 id;
};

#define to_clk_master(_clk) container_of(_clk, struct clk_master, clk)

static inline bool clk_master_ready(struct clk_master *master)
{
	unsigned int status;

	pmc_read(master->base, AT91_PMC_SR, &status);

	return !!(status & AT91_PMC_MCKRDY);
}

static int clk_master_enable(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);

	while (!clk_master_ready(master)) {
		debug("waiting for mck %d\n", master->id);
		cpu_relax();
	}

	return 0;
}

static ulong clk_master_get_rate(struct clk *clk)
{
	struct clk_master *master = to_clk_master(clk);
	const struct clk_master_layout *layout = master->layout;
	const struct clk_master_characteristics *characteristics =
						master->characteristics;
	ulong rate = clk_get_parent_rate(clk);
	unsigned int mckr;
	u8 pres, div;

	if (!rate)
		return 0;

	pmc_read(master->base, master->layout->offset, &mckr);
	mckr &= layout->mask;

	pres = (mckr >> layout->pres_shift) & MASTER_PRES_MASK;
	div = (mckr >> MASTER_DIV_SHIFT) & MASTER_DIV_MASK;

	if (characteristics->have_div3_pres && pres == MASTER_PRES_MAX)
		rate /= 3;
	else
		rate >>= pres;

	rate /= characteristics->divisors[div];

	if (rate < characteristics->output.min)
		pr_warn("master clk is underclocked");
	else if (rate > characteristics->output.max)
		pr_warn("master clk is overclocked");

	return rate;
}

static const struct clk_ops master_ops = {
	.enable = clk_master_enable,
	.get_rate = clk_master_get_rate,
};

struct clk *at91_clk_register_master(void __iomem *base,
		const char *name, const char * const *parent_names,
		int num_parents, const struct clk_master_layout *layout,
		const struct clk_master_characteristics *characteristics,
		const u32 *mux_table)
{
	struct clk_master *master;
	struct clk *clk;
	unsigned int val;
	int ret;

	if (!base || !name || !num_parents || !parent_names ||
	    !layout || !characteristics || !mux_table)
		return ERR_PTR(-EINVAL);

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return ERR_PTR(-ENOMEM);

	master->layout = layout;
	master->characteristics = characteristics;
	master->base = base;
	master->num_parents = num_parents;
	master->mux_table = mux_table;

	pmc_read(master->base, master->layout->offset, &val);
	clk = &master->clk;
	clk->flags = CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_MASTER, name,
			   parent_names[val & AT91_PMC_CSS]);
	if (ret) {
		kfree(master);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_master_clk) = {
	.name = UBOOT_DM_CLK_AT91_MASTER,
	.id = UCLASS_CLK,
	.ops = &master_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

const struct clk_master_layout at91rm9200_master_layout = {
	.mask = 0x31F,
	.pres_shift = 2,
	.offset = AT91_PMC_MCKR,
};

const struct clk_master_layout at91sam9x5_master_layout = {
	.mask = 0x373,
	.pres_shift = 4,
	.offset = AT91_PMC_MCKR,
};
