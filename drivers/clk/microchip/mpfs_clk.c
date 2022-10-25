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
#include <dt-bindings/clock/microchip-mpfs-clock.h>
#include <linux/err.h>

#include "mpfs_clk.h"

static int mpfs_clk_probe(struct udevice *dev)
{
	struct clk *parent_clk = dev_get_priv(dev);
	struct clk clk_ahb = { .id = CLK_AHB };
	void __iomem *base;
	int ret;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, parent_clk);
	if (ret)
		return ret;

	ret = mpfs_clk_register_cfgs(base, parent_clk);
	if (ret)
		return ret;

	clk_request(dev, &clk_ahb);
	ret = mpfs_clk_register_periphs(base, &clk_ahb);

	return ret;
}

static const struct udevice_id mpfs_of_match[] = {
	{ .compatible = "microchip,mpfs-clkcfg" },
	{ }
};

U_BOOT_DRIVER(mpfs_clk) = {
	.name = "mpfs_clk",
	.id = UCLASS_CLK,
	.of_match = mpfs_of_match,
	.ops = &ccf_clk_ops,
	.probe = mpfs_clk_probe,
	.priv_auto = sizeof(struct clk),
	.flags = DM_FLAG_PRE_RELOC,
};
