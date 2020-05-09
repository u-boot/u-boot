// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip PCIe PHY driver
 *
 * Copyright (c) 2016 Rockchip, Inc.
 * Copyright (c) 2020 Amarula Solutions(India)
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <reset.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <asm/arch-rockchip/clock.h>

#include "pcie_rockchip.h"

DECLARE_GLOBAL_DATA_PTR;

static void phy_wr_cfg(struct rockchip_pcie_phy *phy, u32 addr, u32 data)
{
	u32 reg;

	reg = HIWORD_UPDATE_MASK(data, PHY_CFG_DATA_MASK, PHY_CFG_DATA_SHIFT);
	reg |= HIWORD_UPDATE_MASK(addr, PHY_CFG_ADDR_MASK, PHY_CFG_ADDR_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_CONF);

	udelay(1);

	reg = HIWORD_UPDATE_MASK(PHY_CFG_WR_ENABLE,
				 PHY_CFG_WR_MASK,
				 PHY_CFG_WR_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_CONF);

	udelay(1);

	reg = HIWORD_UPDATE_MASK(PHY_CFG_WR_DISABLE,
				 PHY_CFG_WR_MASK,
				 PHY_CFG_WR_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_CONF);
}

static int rockchip_pcie_phy_power_on(struct rockchip_pcie_phy *phy)
{
	int ret = 0;
	u32 reg, status;

	ret = reset_deassert(&phy->phy_rst);
	if (ret) {
		dev_err(dev, "failed to assert phy reset\n");
		return ret;
	}

	reg = HIWORD_UPDATE_MASK(PHY_CFG_PLL_LOCK,
				 PHY_CFG_ADDR_MASK,
				 PHY_CFG_ADDR_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_CONF);

	reg = HIWORD_UPDATE_MASK(!PHY_LANE_IDLE_OFF,
				 PHY_LANE_IDLE_MASK,
				 PHY_LANE_IDLE_A_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_LANEOFF);

	ret = -EINVAL;
	ret = readl_poll_sleep_timeout(phy->reg_base + PCIE_PHY_STATUS,
				       status,
				       status & PHY_PLL_LOCKED,
				       20 * 1000,
				       50);
	if (ret) {
		dev_err(&phy->dev, "pll lock timeout!\n");
		goto err_pll_lock;
	}

	phy_wr_cfg(phy, PHY_CFG_CLK_TEST, PHY_CFG_SEPE_RATE);
	phy_wr_cfg(phy, PHY_CFG_CLK_SCC, PHY_CFG_PLL_100M);

	ret = -ETIMEDOUT;
	ret = readl_poll_sleep_timeout(phy->reg_base + PCIE_PHY_STATUS,
				       status,
				       !(status & PHY_PLL_OUTPUT),
				       20 * 1000,
				       50);
	if (ret) {
		dev_err(&phy->dev, "pll output enable timeout!\n");
		goto err_pll_lock;
	}

	reg = HIWORD_UPDATE_MASK(PHY_CFG_PLL_LOCK,
				 PHY_CFG_ADDR_MASK,
				 PHY_CFG_ADDR_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_CONF);

	ret = -EINVAL;
	ret = readl_poll_sleep_timeout(phy->reg_base + PCIE_PHY_STATUS,
				       status,
				       status & PHY_PLL_LOCKED,
				       20 * 1000,
				       50);
	if (ret) {
		dev_err(&phy->dev, "pll relock timeout!\n");
		goto err_pll_lock;
	}

	return 0;

err_pll_lock:
	reset_assert(&phy->phy_rst);
	return ret;
}

static int rockchip_pcie_phy_power_off(struct rockchip_pcie_phy *phy)
{
	int ret;
	u32 reg;

	reg = HIWORD_UPDATE_MASK(PHY_LANE_IDLE_OFF,
				 PHY_LANE_IDLE_MASK,
				 PHY_LANE_IDLE_A_SHIFT);
	writel(reg, phy->reg_base + PCIE_PHY_LANEOFF);

	ret = reset_assert(&phy->phy_rst);
	if (ret) {
		dev_err(dev, "failed to assert phy reset\n");
		return ret;
	}

	return 0;
}

static int rockchip_pcie_phy_init(struct rockchip_pcie_phy *phy)
{
	int ret;

	ret = clk_enable(&phy->refclk);
	if (ret) {
		dev_err(dev, "failed to enable refclk clock\n");
		return ret;
	}

	ret = reset_assert(&phy->phy_rst);
	if (ret) {
		dev_err(dev, "failed to assert phy reset\n");
		goto err_reset;
	}

	return 0;

err_reset:
	clk_disable(&phy->refclk);
	return ret;
}

static int rockchip_pcie_phy_exit(struct rockchip_pcie_phy *phy)
{
	clk_disable(&phy->refclk);

	return 0;
}

static struct rockchip_pcie_phy_ops pcie_phy_ops = {
	.init = rockchip_pcie_phy_init,
	.power_on = rockchip_pcie_phy_power_on,
	.power_off = rockchip_pcie_phy_power_off,
	.exit = rockchip_pcie_phy_exit,
};

int rockchip_pcie_phy_get(struct udevice *dev)
{
	struct rockchip_pcie *priv = dev_get_priv(dev);
	struct rockchip_pcie_phy *phy_priv = &priv->rk_phy;
	ofnode phy_node;
	u32 phandle;
	int ret;

	phandle = dev_read_u32_default(dev, "phys", 0);
	phy_node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(phy_node)) {
		dev_err(dev, "failed to found pcie-phy\n");
		return -ENODEV;
	}

	phy_priv->reg_base = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	ret = clk_get_by_index_nodev(phy_node, 0, &phy_priv->refclk);
	if (ret) {
		dev_err(dev, "failed to get refclk clock phandle\n");
		return ret;
	}

	ret = reset_get_by_index_nodev(phy_node, 0, &phy_priv->phy_rst);
	if (ret) {
		dev_err(dev, "failed to get phy reset phandle\n");
		return ret;
	}

	phy_priv->ops = &pcie_phy_ops;
	priv->phy = phy_priv;

	return 0;
}
