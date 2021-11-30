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
 * @ref_clk_m: The reference clock of M for PHY
 * @ref_clk_n: The reference clock of N for PHY
 * @pclk: The clock for accessing PHY blocks
 */
struct rockchip_p3phy_priv {
	void __iomem *mmio;
	struct regmap *phy_grf;
	struct reset_ctl p30phy;
	struct clk ref_clk_m;
	struct clk ref_clk_n;
	struct clk pclk;
};

static int rochchip_p3phy_init(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);
	int ret;

	ret = clk_enable(&priv->ref_clk_m);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	ret = clk_enable(&priv->ref_clk_n);
	if (ret < 0 && ret != -ENOSYS)
		goto err_ref;

	ret = clk_enable(&priv->pclk);
	if (ret < 0 && ret != -ENOSYS)
		goto err_pclk;

	reset_assert(&priv->p30phy);
	udelay(1);

	/* Deassert PCIe PMA output clamp mode */
	regmap_write(priv->phy_grf, GRF_PCIE30PHY_CON9,
		     (0x1 << 15) | (0x1 << 31));

	reset_deassert(&priv->p30phy);
	udelay(1);

	return 0;
err_pclk:
	clk_disable(&priv->ref_clk_n);
err_ref:
	clk_disable(&priv->ref_clk_m);

	return ret;
}

static int rochchip_p3phy_exit(struct phy *phy)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(phy->dev);

	clk_disable(&priv->ref_clk_m);
	clk_disable(&priv->ref_clk_n);
	clk_disable(&priv->pclk);
	reset_assert(&priv->p30phy);

	return 0;
}

static int rockchip_p3phy_probe(struct udevice *dev)
{
	struct rockchip_p3phy_priv *priv = dev_get_priv(dev);
	struct udevice *syscon;
	int ret;

	priv->mmio = (void __iomem *)dev_read_addr(dev);
	if ((fdt_addr_t)priv->mmio == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "rockchip,phy-grf",  &syscon);
	if (ret) {
		pr_err("unable to find syscon device for rockchip,phy-grf\n");
		return ret;
	}

	priv->phy_grf = syscon_get_regmap(syscon);
	if (IS_ERR(priv->phy_grf)) {
		dev_err(dev, "failed to find rockchip,phy_grf regmap\n");
		return PTR_ERR(priv->phy_grf);
	}

	ret = reset_get_by_name(dev, "phy", &priv->p30phy);
	if (ret) {
		dev_err(dev, "no phy reset control specified\n");
		return ret;
	}

	ret = clk_get_by_name(dev, "refclk_m", &priv->ref_clk_m);
	if (ret) {
		dev_err(dev, "failed to find ref clock M\n");
		return PTR_ERR(&priv->ref_clk_m);
	}

	ret = clk_get_by_name(dev, "refclk_n", &priv->ref_clk_n);
	if (ret) {
		dev_err(dev, "failed to find ref clock N\n");
		return PTR_ERR(&priv->ref_clk_n);
	}

	ret = clk_get_by_name(dev, "pclk", &priv->pclk);
	if (ret) {
		dev_err(dev, "failed to find pclk\n");
		return PTR_ERR(&priv->pclk);
	}

	return 0;
}

static struct phy_ops rochchip_p3phy_ops = {
	.init = rochchip_p3phy_init,
	.exit = rochchip_p3phy_exit,
};

static const struct udevice_id rockchip_p3phy_of_match[] = {
	{ .compatible = "rockchip,rk3568-pcie3-phy" },
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
