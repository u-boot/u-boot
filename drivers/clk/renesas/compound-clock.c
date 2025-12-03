// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#define LOG_CATEGORY UCLASS_CLK

#include <clk-uclass.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/clk-provider.h>
#include <log.h>

struct clk_compound_rate {
	struct clk clk;		/* This clock */
	struct clk mdlc;	/* MDLC parent module clock */
	struct clk per;		/* Peripheral parent clock */
};

static struct clk_compound_rate *to_clk_compound_rate(struct clk *clk)
{
	return (struct clk_compound_rate *)dev_get_plat(clk->dev);
}

static int clk_compound_rate_enable(struct clk *clk)
{
	struct clk_compound_rate *cc = to_clk_compound_rate(clk);

	return clk_enable(&cc->mdlc);
}

static int clk_compound_rate_disable(struct clk *clk)
{
	struct clk_compound_rate *cc = to_clk_compound_rate(clk);

	return clk_disable(&cc->mdlc);
}

static ulong clk_compound_rate_get_rate(struct clk *clk)
{
	struct clk_compound_rate *cc = to_clk_compound_rate(clk);

	return clk_get_rate(&cc->per);
}

static ulong clk_compound_rate_set_rate(struct clk *clk, ulong rate)
{
	return 0;	/* Set rate is not forwarded to SCP */
}

const struct clk_ops clk_compound_rate_ops = {
	.enable		= clk_compound_rate_enable,
	.disable	= clk_compound_rate_disable,
	.get_rate	= clk_compound_rate_get_rate,
	.set_rate	= clk_compound_rate_set_rate,
};

static int clk_compound_rate_of_to_plat(struct udevice *dev)
{
	struct clk_compound_rate *cc = (struct clk_compound_rate *)dev_get_plat(dev);
	struct clk *clk = &cc->clk;
	int ret;

	clk->dev = dev;
	clk->id = CLK_ID(dev, 0);
	clk->enable_count = 0;

	ret = clk_get_by_index(dev, 0, &cc->mdlc);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 1, &cc->per);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id clk_compound_rate_match[] = {
	{ .compatible = "renesas,compound-clock", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(renesas_compound_clock) = {
	.name = "compound-clock",
	.id = UCLASS_CLK,
	.of_match = clk_compound_rate_match,
	.of_to_plat = clk_compound_rate_of_to_plat,
	.plat_auto	= sizeof(struct clk_compound_rate),
	.ops = &clk_compound_rate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
