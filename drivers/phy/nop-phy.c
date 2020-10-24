// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <generic-phy.h>

struct nop_phy_priv {
	struct clk_bulk bulk;
};

static int nop_phy_init(struct phy *phy)
{
	struct nop_phy_priv *priv = dev_get_priv(phy->dev);

	if (CONFIG_IS_ENABLED(CLK))
		return clk_enable_bulk(&priv->bulk);

	return 0;
}

static int nop_phy_probe(struct udevice *dev)
{
	struct nop_phy_priv *priv = dev_get_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(CLK)) {
		ret = clk_get_bulk(dev, &priv->bulk);
		if (ret < 0) {
			dev_err(dev, "Failed to get clk: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static const struct udevice_id nop_phy_ids[] = {
	{ .compatible = "nop-phy" },
	{ }
};

static struct phy_ops nop_phy_ops = {
	.init = nop_phy_init,
};

U_BOOT_DRIVER(nop_phy) = {
	.name	= "nop_phy",
	.id	= UCLASS_PHY,
	.of_match = nop_phy_ids,
	.ops = &nop_phy_ops,
	.probe = nop_phy_probe,
	.priv_auto_alloc_size = sizeof(struct nop_phy_priv),
};
