// SPDX-License-Identifier: GPL-2.0+
/*
 * Sharp LQ101R1SX01 DSI panel driver
 *
 * Copyright (C) 2014 NVIDIA Corporation
 * Copyright (c) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct sharp_lq101r1sx01_priv {
	struct udevice *backlight;
	struct udevice *panel_sec;
	struct udevice *vcc;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 278000000,
	.hactive.typ		= 2560,
	.hfront_porch.typ	= 128,
	.hback_porch.typ	= 64,
	.hsync_len.typ		= 64,
	.vactive.typ		= 1600,
	.vfront_porch.typ	= 4,
	.vback_porch.typ	= 8,
	.vsync_len.typ		= 32,
};

static int sharp_lq101r1sx01_write(struct mipi_dsi_device *dsi,
				   u16 offset, u8 value)
{
	u8 payload[3] = { offset >> 8, offset & 0xff, value };
	int ret;

	ret = mipi_dsi_generic_write(dsi, payload, sizeof(payload));
	if (ret < 0) {
		log_debug("%s: failed to write %02x to %04x: %zd\n",
			  __func__, value, offset, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_nop(dsi);
	if (ret < 0) {
		log_debug("%s: failed to send DCS nop: %zd\n",
			  __func__, ret);
		return ret;
	}

	udelay(20);

	return 0;
}

static int sharp_setup_symmetrical_split(struct mipi_dsi_device *left,
					 struct mipi_dsi_device *right,
					 struct display_timing *timing)
{
	int ret;

	ret = mipi_dsi_dcs_set_column_address(left, 0,
					      timing->hactive.typ / 2 - 1);
	if (ret < 0) {
		log_debug("%s: failed to set column address: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_page_address(left, 0, timing->vactive.typ - 1);
	if (ret < 0) {
		log_debug("%s: failed to set page address: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_column_address(right, timing->hactive.typ / 2,
					      timing->hactive.typ - 1);
	if (ret < 0) {
		log_debug("%s: failed to set column address: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_page_address(right, 0, timing->vactive.typ - 1);
	if (ret < 0) {
		log_debug("%s: failed to set page address: %d\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int sharp_lq101r1sx01_enable_backlight(struct udevice *dev)
{
	struct sharp_lq101r1sx01_priv *priv = dev_get_priv(dev);

	if (!priv->panel_sec)
		return 0;

	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_panel_plat *plat_sec = dev_get_plat(priv->panel_sec);
	struct mipi_dsi_device *link1 = plat->device;
	struct mipi_dsi_device *link2 = plat_sec->device;
	int ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(link1);
	if (ret < 0) {
		log_debug("%s: failed to exit sleep mode: %d\n",
			  __func__, ret);
		return ret;
	}

	/* set left-right mode */
	ret = sharp_lq101r1sx01_write(link1, 0x1000, 0x2a);
	if (ret < 0) {
		log_debug("%s: failed to set left-right mode: %d\n",
			  __func__, ret);
		return ret;
	}

	/* enable command mode */
	ret = sharp_lq101r1sx01_write(link1, 0x1001, 0x01);
	if (ret < 0) {
		log_debug("%s: failed to enable command mode: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_pixel_format(link1, MIPI_DCS_PIXEL_FMT_24BIT);
	if (ret < 0) {
		log_debug("%s: failed to set pixel format: %d\n",
			  __func__, ret);
		return ret;
	}

	/*
	 * TODO: The device supports both left-right and even-odd split
	 * configurations, but this driver currently supports only the left-
	 * right split. To support a different mode a mechanism needs to be
	 * put in place to communicate the configuration back to the DSI host
	 * controller.
	 */
	ret = sharp_setup_symmetrical_split(link1, link2, &default_timing);
	if (ret < 0) {
		log_debug("%s: failed to set up symmetrical split: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_display_on(link1);
	if (ret < 0) {
		log_debug("%s: failed to set panel on: %d\n",
			  __func__, ret);
		return ret;
	}
	mdelay(20);

	return 0;
}

static int sharp_lq101r1sx01_set_backlight(struct udevice *dev, int percent)
{
	struct sharp_lq101r1sx01_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv->panel_sec)
		return 0;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return backlight_set_brightness(priv->backlight, percent);
}

static int sharp_lq101r1sx01_timings(struct udevice *dev,
				     struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int sharp_lq101r1sx01_of_to_plat(struct udevice *dev)
{
	struct sharp_lq101r1sx01_priv *priv = dev_get_priv(dev);
	int ret;

	/* If node has no link2 it is secondary panel */
	if (!dev_read_bool(dev, "link2"))
		return 0;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL, dev,
					   "link2", &priv->panel_sec);
	if (ret) {
		log_debug("%s: cannot get secondary panel: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_debug("%s: cannot get backlight: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "power-supply", &priv->vcc);
	if (ret) {
		log_debug("%s: cannot get power-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int sharp_lq101r1sx01_hw_init(struct udevice *dev)
{
	struct sharp_lq101r1sx01_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv->panel_sec)
		return 0;

	ret = regulator_set_enable_if_allowed(priv->vcc, 1);
	if (ret) {
		log_debug("%s: enabling power-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	/*
	 * According to the datasheet, the panel needs around 10 ms to fully
	 * power up. At least another 120 ms is required before exiting sleep
	 * mode to make sure the panel is ready. Throw in another 20 ms for
	 * good measure.
	 */
	mdelay(150);

	return 0;
}

static int sharp_lq101r1sx01_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_LPM;

	return sharp_lq101r1sx01_hw_init(dev);
}

static const struct panel_ops sharp_lq101r1sx01_ops = {
	.enable_backlight	= sharp_lq101r1sx01_enable_backlight,
	.set_backlight		= sharp_lq101r1sx01_set_backlight,
	.get_display_timing	= sharp_lq101r1sx01_timings,
};

static const struct udevice_id sharp_lq101r1sx01_ids[] = {
	{ .compatible = "sharp,lq101r1sx01" },
	{ }
};

U_BOOT_DRIVER(sharp_lq101r1sx01) = {
	.name		= "sharp_lq101r1sx01",
	.id		= UCLASS_PANEL,
	.of_match	= sharp_lq101r1sx01_ids,
	.ops		= &sharp_lq101r1sx01_ops,
	.of_to_plat	= sharp_lq101r1sx01_of_to_plat,
	.probe		= sharp_lq101r1sx01_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct sharp_lq101r1sx01_priv),
};
