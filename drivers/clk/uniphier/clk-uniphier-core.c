/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <clk-uclass.h>
#include <dm/device.h>

#include "clk-uniphier.h"

static int uniphier_clk_enable(struct clk *clk)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	struct uniphier_clk_gate_data *gate = priv->socdata->gate;
	unsigned int nr_gate = priv->socdata->nr_gate;
	void __iomem *reg;
	u32 mask, data, tmp;
	int i;

	for (i = 0; i < nr_gate; i++) {
		if (gate[i].index != clk->id)
			continue;

		reg = priv->base + gate[i].reg;
		mask = gate[i].mask;
		data = gate[i].data & mask;

		tmp = readl(reg);
		tmp &= ~mask;
		tmp |= data & mask;
		debug("%s: %p: %08x\n", __func__, reg, tmp);
		writel(tmp, reg);
	}

	return 0;
}

static ulong uniphier_clk_get_rate(struct clk *clk)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	struct uniphier_clk_rate_data *rdata = priv->socdata->rate;
	unsigned int nr_rdata = priv->socdata->nr_rate;
	void __iomem *reg;
	u32 mask, data;
	ulong matched_rate = 0;
	int i;

	for (i = 0; i < nr_rdata; i++) {
		if (rdata[i].index != clk->id)
			continue;

		if (rdata[i].reg == UNIPHIER_CLK_RATE_IS_FIXED)
			return rdata[i].rate;

		reg = priv->base + rdata[i].reg;
		mask = rdata[i].mask;
		data = rdata[i].data & mask;
		if ((readl(reg) & mask) == data) {
			if (matched_rate && rdata[i].rate != matched_rate) {
				printf("failed to get clk rate for insane register values\n");
				return -EINVAL;
			}
			matched_rate = rdata[i].rate;
		}
	}

	debug("%s: rate = %lu\n", __func__, matched_rate);

	return matched_rate;
}

static ulong uniphier_clk_set_rate(struct clk *clk, ulong rate)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	struct uniphier_clk_rate_data *rdata = priv->socdata->rate;
	unsigned int nr_rdata = priv->socdata->nr_rate;
	void __iomem *reg;
	u32 mask, data, tmp;
	ulong best_rate = 0;
	int i;

	/* first, decide the best match rate */
	for (i = 0; i < nr_rdata; i++) {
		if (rdata[i].index != clk->id)
			continue;

		if (rdata[i].reg == UNIPHIER_CLK_RATE_IS_FIXED)
			return 0;

		if (rdata[i].rate > best_rate && rdata[i].rate <= rate)
			best_rate = rdata[i].rate;
	}

	if (!best_rate)
		return -ENODEV;

	debug("%s: requested rate = %lu, set rate = %lu\n", __func__,
	      rate, best_rate);

	/* second, really set registers */
	for (i = 0; i < nr_rdata; i++) {
		if (rdata[i].index != clk->id || rdata[i].rate != best_rate)
			continue;

		reg = priv->base + rdata[i].reg;
		mask = rdata[i].mask;
		data = rdata[i].data & mask;

		tmp = readl(reg);
		tmp &= ~mask;
		tmp |= data;
		debug("%s: %p: %08x\n", __func__, reg, tmp);
		writel(tmp, reg);
	}

	return best_rate;
}

const struct clk_ops uniphier_clk_ops = {
	.enable = uniphier_clk_enable,
	.get_rate = uniphier_clk_get_rate,
	.set_rate = uniphier_clk_set_rate,
};

int uniphier_clk_probe(struct udevice *dev)
{
	struct uniphier_clk_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = devm_ioremap(dev, addr, SZ_4K);
	if (!priv->base)
		return -ENOMEM;

	priv->socdata = (void *)dev_get_driver_data(dev);

	return 0;
}
