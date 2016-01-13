/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <asm/test.h>

struct sandbox_clk_priv {
	ulong rate;
	ulong periph_rate[PERIPH_ID_COUNT];
};

static ulong sandbox_clk_get_rate(struct udevice *dev)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	return priv->rate;
}

static ulong sandbox_clk_set_rate(struct udevice *dev, ulong rate)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	if (!rate)
		return -EINVAL;
	priv->rate = rate;
	return 0;
}

static ulong sandbox_get_periph_rate(struct udevice *dev, int periph)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	if (periph < PERIPH_ID_FIRST || periph >= PERIPH_ID_COUNT)
		return -EINVAL;
	return priv->periph_rate[periph];
}

static ulong sandbox_set_periph_rate(struct udevice *dev, int periph,
				     ulong rate)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);
	ulong old_rate;

	if (periph < PERIPH_ID_FIRST || periph >= PERIPH_ID_COUNT)
		return -EINVAL;
	old_rate = priv->periph_rate[periph];
	priv->periph_rate[periph] = rate;

	return old_rate;
}

static int sandbox_clk_probe(struct udevice *dev)
{
	struct sandbox_clk_priv *priv = dev_get_priv(dev);

	priv->rate = SANDBOX_CLK_RATE;

	return 0;
}

static struct clk_ops sandbox_clk_ops = {
	.get_rate	= sandbox_clk_get_rate,
	.set_rate	= sandbox_clk_set_rate,
	.get_periph_rate = sandbox_get_periph_rate,
	.set_periph_rate = sandbox_set_periph_rate,
};

static const struct udevice_id sandbox_clk_ids[] = {
	{ .compatible = "sandbox,clk" },
	{ }
};

U_BOOT_DRIVER(clk_sandbox) = {
	.name		= "clk_sandbox",
	.id		= UCLASS_CLK,
	.of_match	= sandbox_clk_ids,
	.ops		= &sandbox_clk_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_clk_priv),
	.probe		= sandbox_clk_probe,
};
