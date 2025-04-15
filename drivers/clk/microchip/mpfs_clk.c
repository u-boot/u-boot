// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <regmap.h>
#include <syscon.h>
#include <dt-bindings/clock/microchip-mpfs-clock.h>
#include <linux/err.h>

#include "mpfs_clk.h"

static int mpfs_clk_syscon_probe(struct udevice *dev, void __iomem **msspll_base,
				 struct regmap **regmap)
{
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "microchip,mpfs-mss-top-sysreg");
	if (!ofnode_valid(node))
		return -ENODEV;

	*regmap = syscon_node_to_regmap(node);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	*msspll_base = dev_read_addr_index_ptr(dev, 0);

	return 0;
}

static int mpfs_clk_old_format_probe(struct udevice *dev, void __iomem **msspll_base,
				     struct regmap **regmap)
{
	int ret;

	ret = regmap_init_mem_index(dev_ofnode(dev), regmap, 0);
	if (ret)
		return ret;

	/*
	 * The original devicetrees for mpfs messed up & defined the msspll's
	 * output as a fixed-frequency, 600 MHz clock & used that as the input
	 * for the clock controller node. The msspll is however not a fixed
	 * frequency clock and later devicetrees handled this properly. Check
	 * the devicetree & if it is one of the fixed ones, register the msspll.
	 * Otherwise, skip registering it & pass the reference clock directly
	 * to the cfg clock registration function.
	 */
	*msspll_base = dev_read_addr_index_ptr(dev, 1);

	return 0;
}

static int mpfs_clk_probe(struct udevice *dev)
{
	struct clk *parent_clk = dev_get_priv(dev);
	struct clk clk_msspll = { .id = CLK_MSSPLL };
	struct regmap *regmap;
	void __iomem *msspll_base;
	int ret;

	ret = clk_get_by_index(dev, 0, parent_clk);
	if (ret)
		return ret;

	ret = mpfs_clk_syscon_probe(dev, &msspll_base, &regmap);
	if (ret) {
		ret = mpfs_clk_old_format_probe(dev, &msspll_base, &regmap);
		if (ret)
			return ret;
	}

	if (msspll_base) {
		ret = mpfs_clk_register_msspll(msspll_base, parent_clk);
		if (ret)
			return ret;

		clk_request(dev, &clk_msspll);
		parent_clk = &clk_msspll;
	}

	ret = mpfs_clk_register_cfgs(parent_clk, regmap);
	if (ret)
		return ret;

	ret = mpfs_clk_register_periphs(dev, regmap);

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
