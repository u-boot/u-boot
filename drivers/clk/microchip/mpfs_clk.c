// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */
#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/uclass.h>
#include <linux/err.h>

#include "mpfs_clk.h"

/* All methods are delegated to CCF clocks */

static ulong mpfs_clk_get_rate(struct clk *clk)
{
	struct clk *c;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;
	return clk_get_rate(c);
}

static ulong mpfs_clk_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk *c;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;
	return clk_set_rate(c, rate);
}

static int mpfs_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *c, *p;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;

	err = clk_get_by_id(parent->id, &p);
	if (err)
		return err;

	return clk_set_parent(c, p);
}

static int mpfs_clk_endisable(struct clk *clk, bool enable)
{
	struct clk *c;
	int err = clk_get_by_id(clk->id, &c);

	if (err)
		return err;
	return enable ? clk_enable(c) : clk_disable(c);
}

static int mpfs_clk_enable(struct clk *clk)
{
	return mpfs_clk_endisable(clk, true);
}

static int mpfs_clk_disable(struct clk *clk)
{
	return mpfs_clk_endisable(clk, false);
}

static int mpfs_clk_probe(struct udevice *dev)
{
	int ret;
	void __iomem *base;
	u32 clk_rate;
	const char *parent_clk_name;
	struct clk *clk = dev_get_priv(dev);

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, clk);
	if (ret)
		return ret;

	dev_read_u32(clk->dev, "clock-frequency", &clk_rate);
	parent_clk_name = clk->dev->name;

	ret = mpfs_clk_register_cfgs(base, clk_rate, parent_clk_name);
	if (ret)
		return ret;

	ret = mpfs_clk_register_periphs(base, clk_rate, "clk_ahb");

	return ret;
}

static const struct clk_ops mpfs_clk_ops = {
	.set_rate = mpfs_clk_set_rate,
	.get_rate = mpfs_clk_get_rate,
	.set_parent = mpfs_clk_set_parent,
	.enable = mpfs_clk_enable,
	.disable = mpfs_clk_disable,
};

static const struct udevice_id mpfs_of_match[] = {
	{ .compatible = "microchip,mpfs-clkcfg" },
	{ }
};

U_BOOT_DRIVER(mpfs_clk) = {
	.name = "mpfs_clk",
	.id = UCLASS_CLK,
	.of_match = mpfs_of_match,
	.ops = &mpfs_clk_ops,
	.probe = mpfs_clk_probe,
	.priv_auto = sizeof(struct clk),
};
