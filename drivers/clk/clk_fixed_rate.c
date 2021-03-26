// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/clk-provider.h>

static ulong clk_fixed_rate_get_rate(struct clk *clk)
{
	return to_clk_fixed_rate(clk->dev)->fixed_rate;
}

/* avoid clk_enable() return -ENOSYS */
static int dummy_enable(struct clk *clk)
{
	return 0;
}

const struct clk_ops clk_fixed_rate_ops = {
	.get_rate = clk_fixed_rate_get_rate,
	.enable = dummy_enable,
};

void clk_fixed_rate_ofdata_to_plat_(struct udevice *dev,
				    struct clk_fixed_rate *plat)
{
	struct clk *clk = &plat->clk;
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	plat->fixed_rate = dev_read_u32_default(dev, "clock-frequency", 0);
#endif
	/* Make fixed rate clock accessible from higher level struct clk */
	/* FIXME: This is not allowed */
	dev_set_uclass_priv(dev, clk);

	clk->dev = dev;
	clk->enable_count = 0;
}

static int clk_fixed_rate_of_to_plat(struct udevice *dev)
{
	clk_fixed_rate_ofdata_to_plat_(dev, to_clk_fixed_rate(dev));

	return 0;
}

static const struct udevice_id clk_fixed_rate_match[] = {
	{
		.compatible = "fixed-clock",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(fixed_clock) = {
	.name = "fixed_clock",
	.id = UCLASS_CLK,
	.of_match = clk_fixed_rate_match,
	.of_to_plat = clk_fixed_rate_of_to_plat,
	.plat_auto	= sizeof(struct clk_fixed_rate),
	.ops = &clk_fixed_rate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
