// SPDX-License-Identifier: GPL-2.0
/*
 * Rockchip PCIE3.0 phy driver
 *
 * Copyright (C) 2021 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <reset-uclass.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>

#define GRF_PCIE30PHY_CON1 0x4
#define GRF_PCIE30PHY_CON6 0x18
#define GRF_PCIE30PHY_CON9 0x24

/**
 * struct rockchip_p3phy_priv - RK DW PCIe PHY state
 *
 * @mmio: The base address of PHY internal registers
 * @phy_grf: The regmap for controlling pipe signal
 * @p30phy: The reset signal for PHY
 * @clks: The clocks for PHY
 */
struct rockchip_p3phy_priv {
	void __iomem *mmio;
	struct regmap *phy_grf;
	struct reset_ctl p30phy;
	struct clk_bulk clks;
};

struct rockchip_p3phy_ops {
	int (*phy_init)(struct phy *phy);
};

static int rockchip_p3phy_rk3568_init(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);

	/* Deassert PCIe PMA output clamp mode */
	regmap_write(priv->phy_grf, GRF_PCIE30PHY_CON9,
		     (0x1 << 15) | (0x1 << 31));

	reset_deassert(&priv->p30phy);
	udelay(1);

	return 0;
}

static const struct rockchip_p3phy_ops rk3568_ops = {
	.phy_init = rockchip_p3phy_rk3568_init,
};

static int rochchip_p3phy_init(struct phy *phy)
{
	struct rockchip_p3phy_ops *ops =
		(struct rockchip_p3phy_ops *)dev_get_driver_data(phy->dev);
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		return ret;

	reset_assert(&priv->p30phy);
	udelay(1);

	ret = ops->phy_init(phy);
	if (ret)
		clk_disable_bulk(&priv->clks);

	return ret;
}

static int rochchip_p3phy_exit(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);

	clk_disable_bulk(&priv->clks);
	reset_assert(&priv->p30phy);

	return 0;
}

static int rockchip_p3phy_probe(struct udevice *dev)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(dev);
	int ret;

	priv->mmio = dev_read_addr_ptr(dev);
	if (!priv->mmio)
		return -EINVAL;

	priv->phy_grf = syscon_regmap_lookup_by_phandle(dev, "rockchip,phy-grf");
	if (IS_ERR(priv->phy_grf)) {
		dev_err(dev, "failed to find rockchip,phy_grf regmap\n");
		return PTR_ERR(priv->phy_grf);
	}

	ret = reset_get_by_name(dev, "phy", &priv->p30phy);
	if (ret) {
		dev_err(dev, "no phy reset control specified\n");
		return ret;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "failed to get clocks\n");
		return ret;
	}

	return 0;
}

static struct phy_ops rochchip_p3phy_ops = {
	.init = rochchip_p3phy_init,
	.exit = rochchip_p3phy_exit,
};

static const struct udevice_id rockchip_p3phy_of_match[] = {
	{
		.compatible = "rockchip,rk3568-pcie3-phy",
		.data = (ulong)&rk3568_ops,
	},
	{ },
};

U_BOOT_DRIVER(rockchip_pcie3phy) = {
	.name		= "rockchip_pcie3phy",
	.id		= UCLASS_PHY,
	.of_match	= rockchip_p3phy_of_match,
	.ops		= &rochchip_p3phy_ops,
	.probe		= rockchip_p3phy_probe,
	.priv_auto	= sizeof(struct rockchip_p3phy_priv),
};
