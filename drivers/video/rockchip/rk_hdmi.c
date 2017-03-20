/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <dw_hdmi.h>
#include <edid.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3288.h>
#include <power/regulator.h>

struct rk_hdmi_priv {
	struct dw_hdmi hdmi;
	struct rk3288_grf *grf;
};

static const struct hdmi_phy_config rockchip_phy_config[] = {
	{
		.mpixelclock = 74250000,
		.sym_ctr = 0x8009, .term = 0x0004, .vlev_ctr = 0x0272,
	}, {
		.mpixelclock = 148500000,
		.sym_ctr = 0x802b, .term = 0x0004, .vlev_ctr = 0x028d,
	}, {
		.mpixelclock = 297000000,
		.sym_ctr = 0x8039, .term = 0x0005, .vlev_ctr = 0x028d,
	}, {
		.mpixelclock = ~0ul,
		.sym_ctr = 0x0000, .term = 0x0000, .vlev_ctr = 0x0000,
	}
};

static const struct hdmi_mpll_config rockchip_mpll_cfg[] = {
	{
		.mpixelclock = 40000000,
		.cpce = 0x00b3, .gmp = 0x0000, .curr = 0x0018,
	}, {
		.mpixelclock = 65000000,
		.cpce = 0x0072, .gmp = 0x0001, .curr = 0x0028,
	}, {
		.mpixelclock = 66000000,
		.cpce = 0x013e, .gmp = 0x0003, .curr = 0x0038,
	}, {
		.mpixelclock = 83500000,
		.cpce = 0x0072, .gmp = 0x0001, .curr = 0x0028,
	}, {
		.mpixelclock = 146250000,
		.cpce = 0x0051, .gmp = 0x0002, .curr = 0x0038,
	}, {
		.mpixelclock = 148500000,
		.cpce = 0x0051, .gmp = 0x0003, .curr = 0x0000,
	}, {
		.mpixelclock = ~0ul,
		.cpce = 0x0051, .gmp = 0x0003, .curr = 0x0000,
	}
};

static int rk_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);

	return dw_hdmi_read_edid(&priv->hdmi, buf, buf_size);
}

static int rk_hdmi_enable(struct udevice *dev, int panel_bpp,
			  const struct display_timing *edid)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);

	return dw_hdmi_enable(&priv->hdmi, edid);
}

static int rk_hdmi_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;

	hdmi->ioaddr = (ulong)dev_get_addr(dev);
	hdmi->mpll_cfg = rockchip_mpll_cfg;
	hdmi->phy_cfg = rockchip_phy_config;
	hdmi->i2c_clk_high = 0x7a;
	hdmi->i2c_clk_low = 0x8d;

	/*
	 * TODO(sjg@chromium.org): The above values don't work - these ones
	 * work better, but generate lots of errors in the data.
	 */
	hdmi->i2c_clk_high = 0x0d;
	hdmi->i2c_clk_low = 0x0d;
	hdmi->reg_io_width = 4;
	hdmi->phy_set = dw_hdmi_phy_cfg;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	return 0;
}

static int rk_hdmi_probe(struct udevice *dev)
{
	struct display_plat *uc_plat = dev_get_uclass_platdata(dev);
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct dw_hdmi *hdmi = &priv->hdmi;
	struct udevice *reg;
	struct clk clk;
	int ret;
	int vop_id = uc_plat->source_id;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 0);
		clk_free(&clk);
	}
	if (ret) {
		debug("%s: Failed to set hdmi clock: ret=%d\n", __func__, ret);
		return ret;
	}

	/*
	 * Configure the maximum clock to permit whatever resolution the
	 * monitor wants
	 */
	ret = clk_get_by_index(uc_plat->src_dev, 0, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 384000000);
		clk_free(&clk);
	}
	if (ret < 0) {
		debug("%s: Failed to set clock in source device '%s': ret=%d\n",
		      __func__, uc_plat->src_dev->name, ret);
		return ret;
	}

	ret = regulator_get_by_platname("vcc50_hdmi", &reg);
	if (!ret)
		ret = regulator_set_enable(reg, true);
	if (ret)
		debug("%s: Cannot set regulator vcc50_hdmi\n", __func__);

	/* hdmi source select hdmi controller */
	rk_setreg(&priv->grf->soc_con6, 1 << 15);

	/* hdmi data from vop id */
	rk_clrsetreg(&priv->grf->soc_con6, 1 << 4,
		     (vop_id == 1) ? (1 << 4) : 0);

	ret = dw_hdmi_phy_wait_for_hpd(hdmi);
	if (ret < 0) {
		debug("hdmi can not get hpd signal\n");
		return -1;
	}

	dw_hdmi_init(hdmi);
	dw_hdmi_phy_init(hdmi);

	return 0;
}

static const struct dm_display_ops rk_hdmi_ops = {
	.read_edid = rk_hdmi_read_edid,
	.enable = rk_hdmi_enable,
};

static const struct udevice_id rk_hdmi_ids[] = {
	{ .compatible = "rockchip,rk3288-dw-hdmi" },
	{ }
};

U_BOOT_DRIVER(hdmi_rockchip) = {
	.name	= "hdmi_rockchip",
	.id	= UCLASS_DISPLAY,
	.of_match = rk_hdmi_ids,
	.ops	= &rk_hdmi_ops,
	.ofdata_to_platdata	= rk_hdmi_ofdata_to_platdata,
	.probe	= rk_hdmi_probe,
	.priv_auto_alloc_size	 = sizeof(struct rk_hdmi_priv),
};
