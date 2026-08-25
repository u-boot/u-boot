// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 * Author: Jie Qiu <jie.qiu@mediatek.com>
 */

#include <dm.h>
#include <dm/device_compat.h>

#include "phy-mtk-hdmi.h"

static int mtk_hdmi_phy_power_on(struct phy *phy)
{
	struct mtk_hdmi_phy *hdmi_phy = dev_get_priv(phy->dev);
	int ret;

	/*
	 * The 5V output is a regulator in the Linux driver, controlled by the
	 * HDMI connector. U-Boot has no such consumer, so enable it here.
	 */
	if (hdmi_phy->conf->power_control)
		hdmi_phy->conf->power_control(hdmi_phy, true);

	/* Equivalent of clk_prepare_enable() on the kernel PLL clock. */
	ret = hdmi_phy->conf->pll_prepare(hdmi_phy);
	if (ret)
		return ret;

	hdmi_phy->conf->hdmi_phy_enable_tmds(hdmi_phy);

	return 0;
}

static int mtk_hdmi_phy_power_off(struct phy *phy)
{
	struct mtk_hdmi_phy *hdmi_phy = dev_get_priv(phy->dev);

	hdmi_phy->conf->hdmi_phy_disable_tmds(hdmi_phy);
	hdmi_phy->conf->pll_unprepare(hdmi_phy);

	if (hdmi_phy->conf->power_control)
		hdmi_phy->conf->power_control(hdmi_phy, false);

	return 0;
}

static int mtk_hdmi_phy_configure(struct phy *phy, void *opts)
{
	struct mtk_hdmi_phy *hdmi_phy = dev_get_priv(phy->dev);

	if (hdmi_phy->conf->hdmi_phy_configure)
		return hdmi_phy->conf->hdmi_phy_configure(phy, opts);

	return 0;
}

static int mtk_hdmi_phy_probe(struct udevice *dev)
{
	struct mtk_hdmi_phy *hdmi_phy = dev_get_priv(dev);
	int ret;

	hdmi_phy->dev = dev;
	hdmi_phy->conf =
		(const struct mtk_hdmi_phy_conf *)dev_get_driver_data(dev);

	hdmi_phy->regs = dev_read_addr_ptr(dev);
	if (!hdmi_phy->regs)
		return -EINVAL;

	ret = clk_get_by_name(dev, "pll_ref", &hdmi_phy->pll_ref);
	if (ret) {
		dev_err(dev, "Failed to get PLL reference clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&hdmi_phy->pll_ref);
	if (ret) {
		dev_err(dev, "Failed to enable PLL reference clock: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct phy_ops mtk_hdmi_phy_ops = {
	.power_on	= mtk_hdmi_phy_power_on,
	.power_off	= mtk_hdmi_phy_power_off,
	.configure	= mtk_hdmi_phy_configure,
};

static const struct udevice_id mtk_hdmi_phy_match[] = {
	{ .compatible = "mediatek,mt8195-hdmi-phy",
	  .data = (ulong)&mtk_hdmi_phy_8195_conf,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mtk_hdmi_phy) = {
	.name		= "mtk-hdmi-phy",
	.id		= UCLASS_PHY,
	.of_match	= mtk_hdmi_phy_match,
	.ops		= &mtk_hdmi_phy_ops,
	.probe		= mtk_hdmi_phy_probe,
	.priv_auto	= sizeof(struct mtk_hdmi_phy),
};
