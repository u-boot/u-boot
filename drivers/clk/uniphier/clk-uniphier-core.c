/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "clk-uniphier.h"

/**
 * struct uniphier_clk_priv - private data for UniPhier clock driver
 *
 * @base: base address of the clock provider
 * @data: SoC specific data
 */
struct uniphier_clk_priv {
	void __iomem *base;
	const struct uniphier_clk_data *data;
};

static int uniphier_clk_enable(struct clk *clk)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	unsigned long id = clk->id;
	const struct uniphier_clk_gate_data *p;

	for (p = priv->data->gate; p->id != UNIPHIER_CLK_ID_END; p++) {
		u32 val;

		if (p->id != id)
			continue;

		val = readl(priv->base + p->reg);
		val |= BIT(p->bit);
		writel(val, priv->base + p->reg);

		return 0;
	}

	dev_err(priv->dev, "clk_id=%lu was not handled\n", id);
	return -EINVAL;
}

static const struct uniphier_clk_mux_data *
uniphier_clk_get_mux_data(struct uniphier_clk_priv *priv, unsigned long id)
{
	const struct uniphier_clk_mux_data *p;

	for (p = priv->data->mux; p->id != UNIPHIER_CLK_ID_END; p++) {
		if (p->id == id)
			return p;
	}

	return NULL;
}

static ulong uniphier_clk_get_rate(struct clk *clk)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	const struct uniphier_clk_mux_data *mux;
	u32 val;
	int i;

	mux = uniphier_clk_get_mux_data(priv, clk->id);
	if (!mux)
		return 0;

	if (!mux->nr_muxs)		/* fixed-rate */
		return mux->rates[0];

	val = readl(priv->base + mux->reg);

	for (i = 0; i < mux->nr_muxs; i++)
		if ((mux->masks[i] & val) == mux->vals[i])
			return mux->rates[i];

	return -EINVAL;
}

static ulong uniphier_clk_set_rate(struct clk *clk, ulong rate)
{
	struct uniphier_clk_priv *priv = dev_get_priv(clk->dev);
	const struct uniphier_clk_mux_data *mux;
	u32 val;
	int i, best_rate_id = -1;
	ulong best_rate = 0;

	mux = uniphier_clk_get_mux_data(priv, clk->id);
	if (!mux)
		return 0;

	if (!mux->nr_muxs)		/* fixed-rate */
		return mux->rates[0];

	/* first, decide the best match rate */
	for (i = 0; i < mux->nr_muxs; i++) {
		if (mux->rates[i] > best_rate && mux->rates[i] <= rate) {
			best_rate = mux->rates[i];
			best_rate_id = i;
		}
	}

	if (best_rate_id < 0)
		return -EINVAL;

	val = readl(priv->base + mux->reg);
	val &= ~mux->masks[best_rate_id];
	val |= mux->vals[best_rate_id];
	writel(val, priv->base + mux->reg);

	debug("%s: requested rate = %lu, set rate = %lu\n", __func__,
	      rate, best_rate);

	return best_rate;
}

static const struct clk_ops uniphier_clk_ops = {
	.enable = uniphier_clk_enable,
	.get_rate = uniphier_clk_get_rate,
	.set_rate = uniphier_clk_set_rate,
};

static int uniphier_clk_probe(struct udevice *dev)
{
	struct uniphier_clk_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = devm_ioremap(dev, addr, SZ_4K);
	if (!priv->base)
		return -ENOMEM;

	priv->data = (void *)dev_get_driver_data(dev);

	return 0;
}

static const struct udevice_id uniphier_clk_match[] = {
	{
		.compatible = "socionext,uniphier-sld3-mio-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld4-mio-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pro4-mio-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-sld8-mio-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pro5-sd-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-sd-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-mio-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-sd-clock",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_clk) = {
	.name = "uniphier-clk",
	.id = UCLASS_CLK,
	.of_match = uniphier_clk_match,
	.probe = uniphier_clk_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_clk_priv),
	.ops = &uniphier_clk_ops,
};
