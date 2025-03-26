// SPDX-License-Identifier: GPL-2.0+
/*
 * LG LD070WX3-SL01 DSI panel driver
 *
 * Copyright (c) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct lg_ld070wx3_priv {
	struct udevice *vdd;
	struct udevice *vcc;

	struct udevice *backlight;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 70000000,
	.hactive.typ		= 800,
	.hfront_porch.typ	= 32,
	.hback_porch.typ	= 48,
	.hsync_len.typ		= 8,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 5,
	.vback_porch.typ	= 3,
	.vsync_len.typ		= 1,
};

static void dcs_write_one(struct mipi_dsi_device *dsi, u8 cmd, u8 data)
{
	mipi_dsi_dcs_write(dsi, cmd, &data, 1);
}

static int lg_ld070wx3_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	ret = mipi_dsi_dcs_soft_reset(dsi);
	if (ret < 0) {
		log_debug("%s: failed to soft reset panel: %d\n",
			  __func__, ret);
		return ret;
	}

	/* Delay before sending new command after soft reset */
	mdelay(20);

	/* Differential input impedance selection */
	dcs_write_one(dsi, 0xAE, 0x0B);

	/* Enter test mode 1 and 2*/
	dcs_write_one(dsi, 0xEE, 0xEA);
	dcs_write_one(dsi, 0xEF, 0x5F);

	/* Increased MIPI CLK driving ability */
	dcs_write_one(dsi, 0xF2, 0x68);

	/* Exit test mode 1 and 2 */
	dcs_write_one(dsi, 0xEE, 0x00);
	dcs_write_one(dsi, 0xEF, 0x00);

	return 0;
}

static int lg_ld070wx3_set_backlight(struct udevice *dev, int percent)
{
	struct lg_ld070wx3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return backlight_set_brightness(priv->backlight, percent);
}

static int lg_ld070wx3_timings(struct udevice *dev,
			       struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int lg_ld070wx3_of_to_plat(struct udevice *dev)
{
	struct lg_ld070wx3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_debug("%s: cannot get backlight: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vdd-supply", &priv->vdd);
	if (ret) {
		log_debug("%s: cannot get vdd-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vcc-supply", &priv->vcc);
	if (ret) {
		log_debug("%s: cannot get vcc-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int lg_ld070wx3_hw_init(struct udevice *dev)
{
	struct lg_ld070wx3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->vcc, 1);
	if (ret) {
		log_debug("%s: enabling vcc-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->vdd, 1);
	if (ret) {
		log_debug("%s: enabling vdd-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	/*
	 * According to spec delay between enabling supply is 0,
	 * for regulators to reach required voltage ~5ms needed.
	 * MIPI interface signal for setup requires additional
	 * 110ms which in total results in 115ms.
	 */
	mdelay(115);

	return 0;
}

static int lg_ld070wx3_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM;

	return lg_ld070wx3_hw_init(dev);
}

static const struct panel_ops lg_ld070wx3_ops = {
	.enable_backlight	= lg_ld070wx3_enable_backlight,
	.set_backlight		= lg_ld070wx3_set_backlight,
	.get_display_timing	= lg_ld070wx3_timings,
};

static const struct udevice_id lg_ld070wx3_ids[] = {
	{ .compatible = "lg,ld070wx3-sl01" },
	{ }
};

U_BOOT_DRIVER(lg_ld070wx3) = {
	.name		= "lg_ld070wx3",
	.id		= UCLASS_PANEL,
	.of_match	= lg_ld070wx3_ids,
	.ops		= &lg_ld070wx3_ops,
	.of_to_plat	= lg_ld070wx3_of_to_plat,
	.probe		= lg_ld070wx3_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct lg_ld070wx3_priv),
};
