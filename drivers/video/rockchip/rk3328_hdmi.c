// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Edgeble AI Technologies Pvt. Ltd.
 */

#include <clk.h>
#include <display.h>
#include <dm.h>
#include <dw_hdmi.h>
#include <asm/io.h>
#include <asm/arch-rockchip/grf_rk3328.h>
#include "rk_hdmi.h"

#define RK3328_IO_3V_DOMAIN              (7 << (9 + 16))
#define RK3328_IO_5V_DOMAIN              ((7 << 9) | (3 << (9 + 16)))
#define RK3328_IO_DDC_IN_MSK             ((3 << 10) | (3 << (10 + 16)))
#define RK3328_IO_CTRL_BY_HDMI           ((1 << 13) | (1 << (13 + 16)))

static int rk3328_hdmi_enable(struct udevice *dev, int panel_bpp,
			      const struct display_timing *edid)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);

	return dw_hdmi_enable(&priv->hdmi, edid);
}

static int rk3328_dw_hdmi_phy_cfg(struct dw_hdmi *hdmi, uint pixclock)
{
	struct rk_hdmi_priv *priv = container_of(hdmi, struct rk_hdmi_priv, hdmi);
	int ret;

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		printf("failed to init phy (ret=%d)\n", ret);
		return ret;
	}

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		printf("failed to power on phy (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

static void rk3328_dw_hdmi_setup_hpd(struct dw_hdmi *hdmi)
{
	struct rk_hdmi_priv *priv = container_of(hdmi, struct rk_hdmi_priv, hdmi);
	struct rk3328_grf_regs *grf = priv->grf;

	writel(RK3328_IO_DDC_IN_MSK, &grf->soc_con[2]);
	writel(RK3328_IO_CTRL_BY_HDMI, &grf->soc_con[3]);
}

static void rk3328_dw_hdmi_read_hpd(struct dw_hdmi *hdmi, bool hpd_status)
{
	struct rk_hdmi_priv *priv = container_of(hdmi, struct rk_hdmi_priv, hdmi);
	struct rk3328_grf_regs *grf = priv->grf;

	if (hpd_status)
		writel(RK3328_IO_5V_DOMAIN, &grf->soc_con[4]);
	else
		writel(RK3328_IO_3V_DOMAIN, &grf->soc_con[4]);
}

static const struct dw_hdmi_phy_ops dw_hdmi_rk3328_phy_ops = {
	.phy_set = rk3328_dw_hdmi_phy_cfg,
	.setup_hpd = rk3328_dw_hdmi_setup_hpd,
	.read_hpd = rk3328_dw_hdmi_read_hpd,
};

static int rk3328_hdmi_of_to_plat(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;

	hdmi->i2c_clk_high = 0x71;
	hdmi->i2c_clk_low = 0x76;

	rk_hdmi_of_to_plat(dev);

	hdmi->ops = &dw_hdmi_rk3328_phy_ops;

	return 0;
}

static int rk3328_hdmi_probe(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	int ret;

	ret = generic_phy_get_by_name(dev, "hdmi", &priv->phy);
	if (ret) {
		printf("failed to get hdmi phy\n");
		return ret;
	};

	ret = rk_hdmi_probe(dev);
	if (ret) {
		printf("failed to probe rk hdmi\n");
		return ret;
	}

	return 0;
}

static const struct dm_display_ops rk3328_hdmi_ops = {
	.read_edid = rk_hdmi_read_edid,
	.enable = rk3328_hdmi_enable,
};

static const struct udevice_id rk3328_hdmi_ids[] = {
	{ .compatible = "rockchip,rk3328-dw-hdmi" },
	{ }
};

U_BOOT_DRIVER(rk3328_hdmi_rockchip) = {
	.name = "rk3328_hdmi_rockchip",
	.id = UCLASS_DISPLAY,
	.of_match = rk3328_hdmi_ids,
	.ops = &rk3328_hdmi_ops,
	.of_to_plat = rk3328_hdmi_of_to_plat,
	.probe = rk3328_hdmi_probe,
	.priv_auto	= sizeof(struct rk_hdmi_priv),
};
