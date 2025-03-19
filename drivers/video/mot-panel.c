// SPDX-License-Identifier: GPL-2.0+
/*
 * Motorola ATRIX 4G and DROID X2 DSI panel driver
 * Exact manufacturer and model unknown
 *
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <misc.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <power/regulator.h>

struct mot_panel_priv {
	struct udevice *vdd;
	struct udevice *vddio;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;
};

#define dsi_generic_write_seq(dsi, cmd, seq...) do {			\
		static const u8 b[] = { cmd, seq };			\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, b, ARRAY_SIZE(b));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static struct display_timing default_timing = {
	.pixelclock.typ		= 38250000,
	.hactive.typ		= 540,
	.hfront_porch.typ	= 32,
	.hback_porch.typ	= 32,
	.hsync_len.typ		= 16,
	.vactive.typ		= 960,
	.vfront_porch.typ	= 12,
	.vback_porch.typ	= 12,
	.vsync_len.typ		= 8,
};

static int mot_es2(struct mipi_dsi_device *dsi)
{
	int ret;

	dsi_generic_write_seq(dsi, 0x55, 0x01);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		log_debug("%s: failed to exit sleep mode: %d\n", __func__, ret);
		return ret;
	}
	mdelay(120);

	dsi_generic_write_seq(dsi, 0xf4, 0x00, 0xbb, 0x46, 0x53, 0x0c, 0x49,
			      0x74, 0x29, 0x12, 0x15, 0x2f, 0x2f, 0x04);
	dsi_generic_write_seq(dsi, 0xf8, 0x4b, 0x04, 0x10, 0x1a, 0x2c, 0x2c,
			      0x2c, 0x2c, 0x14, 0x12);

	dsi_generic_write_seq(dsi, 0xb5, 0x03, 0x7f, 0x00, 0x80, 0xc7, 0x00);
	dsi_generic_write_seq(dsi, 0xb7, 0x66, 0xf6, 0x46, 0x9f, 0x90, 0x99,
			      0xff, 0x80, 0x6d, 0x01);

	/* Gamma R */
	dsi_generic_write_seq(dsi, 0xf9, 0x04);
	dsi_generic_write_seq(dsi, 0xfa, 0x00, 0x2f, 0x30, 0x12, 0x0e, 0x0c,
			      0x22, 0x27, 0x31, 0x2e, 0x07, 0x0f);
	dsi_generic_write_seq(dsi, 0xfb, 0x00, 0x2f, 0x30, 0x12, 0x0e, 0x0c,
			      0x22, 0x27, 0x31, 0x2e, 0x07, 0x0f);

	/* Gamma G */
	dsi_generic_write_seq(dsi, 0xf9, 0x02);
	dsi_generic_write_seq(dsi, 0xfa, 0x00, 0x2f, 0x37, 0x15, 0x15, 0x11,
			      0x1f, 0x25, 0x2d, 0x2a, 0x05, 0x0f);
	dsi_generic_write_seq(dsi, 0xfb, 0x00, 0x2f, 0x37, 0x15, 0x15, 0x11,
			      0x1f, 0x25, 0x2d, 0x2a, 0x05, 0x0f);

	/* Gamma B */
	dsi_generic_write_seq(dsi, 0xf9, 0x01);
	dsi_generic_write_seq(dsi, 0xfa, 0x00, 0x2f, 0x3f, 0x16, 0x1f, 0x15,
			      0x1f, 0x25, 0x2d, 0x2b, 0x06, 0x0b);
	dsi_generic_write_seq(dsi, 0xfb, 0x00, 0x2f, 0x3f, 0x16, 0x1f, 0x15,
			      0x1f, 0x25, 0x2d, 0x2b, 0x06, 0x0b);

	/* Gamma W */
	dsi_generic_write_seq(dsi, 0xf9, 0x20);
	dsi_generic_write_seq(dsi, 0xfa, 0x00, 0x2f, 0x34, 0x15, 0x1a, 0x11,
			      0x1f, 0x23, 0x2d, 0x29, 0x02, 0x08);
	dsi_generic_write_seq(dsi, 0xfb, 0x00, 0x2f, 0x34, 0x15, 0x1a, 0x11,
			      0x1f, 0x23, 0x2d, 0x29, 0x02, 0x08);

	dsi_generic_write_seq(dsi, 0x53, 0x2c);
	dsi_generic_write_seq(dsi, 0x35, 0x00);

	return 0;
}

static int __maybe_unused mot_es4(struct mipi_dsi_device *dsi)
{
	int ret;

	dsi_generic_write_seq(dsi, 0xd2, 0x04, 0x53);
	dsi_generic_write_seq(dsi, 0xd2, 0x05, 0x53);
	dsi_generic_write_seq(dsi, 0x55, 0x01);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		log_debug("%s: failed to exit sleep mode: %d\n", __func__, ret);
		return ret;
	}
	mdelay(120);

	dsi_generic_write_seq(dsi, 0xb5, 0x03, 0x7f, 0x0a, 0x80, 0xff, 0x00);
	dsi_generic_write_seq(dsi, 0xb7, 0x7a, 0xf7, 0x4d, 0x91, 0x90, 0xb3,
			      0xff, 0x80, 0x6d, 0x01);
	dsi_generic_write_seq(dsi, 0xf4, 0x00, 0xbb, 0x46, 0x53, 0x0c, 0x49,
			      0x74, 0x29, 0x12, 0x15, 0x37, 0x37, 0x04);
	dsi_generic_write_seq(dsi, 0xf8, 0x0a, 0x04, 0x10, 0x2a, 0x35, 0x35,
			      0x35, 0x35, 0x21, 0x1a);

	/* Gamma R */
	dsi_generic_write_seq(dsi, 0xf9, 0x04);
	dsi_generic_write_seq(dsi, 0xfa, 0x08, 0x1c, 0x1b, 0x0f, 0x0f, 0x0a,
			      0x1e, 0x22, 0x27, 0x26, 0x07, 0x0d);
	dsi_generic_write_seq(dsi, 0xfb, 0x08, 0x3c, 0x27, 0x0f, 0x0f, 0x0a,
			      0x1e, 0x26, 0x31, 0x2f, 0x07, 0x0b);

	/* Gamma G */
	dsi_generic_write_seq(dsi, 0xf9, 0x02);
	dsi_generic_write_seq(dsi, 0xfa, 0x30, 0x14, 0x0f, 0x00, 0x06, 0x02,
			      0x1e, 0x22, 0x27, 0x27, 0x08, 0x10);
	dsi_generic_write_seq(dsi, 0xfb, 0x30, 0x35, 0x0f, 0x00, 0x0a, 0x02,
			      0x1c, 0x23, 0x31, 0x2f, 0x08, 0x0e);

	/* Gamma B */
	dsi_generic_write_seq(dsi, 0xf9, 0x01);
	dsi_generic_write_seq(dsi, 0xfa, 0x12, 0x1b, 0x26, 0x0e, 0x12, 0x0b,
			      0x1e, 0x22, 0x27, 0x27, 0x06, 0x0c);
	dsi_generic_write_seq(dsi, 0xfb, 0x12, 0x3b, 0x2c, 0x12, 0x12, 0x0e,
			      0x1e, 0x26, 0x31, 0x2f, 0x06, 0x0d);

	/* Gamma W */
	dsi_generic_write_seq(dsi, 0xf9, 0x20);
	dsi_generic_write_seq(dsi, 0xfa, 0x37, 0x1b, 0x09, 0x01, 0x06, 0x04,
			      0x19, 0x19, 0x22, 0x24, 0x04, 0x15);
	dsi_generic_write_seq(dsi, 0xfb, 0x37, 0x3b, 0x17, 0x01, 0x0a, 0x04,
			      0x19, 0x1d, 0x2c, 0x2c, 0x04, 0x13);

	dsi_generic_write_seq(dsi, 0x53, 0x2c);
	dsi_generic_write_seq(dsi, 0x35, 0x00);
	dsi_generic_write_seq(dsi, 0xc3, 0x01, 0x4e);

	return 0;
}

static int mot_panel_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	dsi_generic_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	dsi_generic_write_seq(dsi, 0xf1, 0x5a, 0x5a);
	dsi_generic_write_seq(dsi, 0xd0, 0x8e);

	ret = mot_es2(dsi);
	if (ret)
		return ret;

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		log_debug("%s: failed to set display on: %d\n", __func__, ret);
		return ret;
	}
	mdelay(20);

	return 0;
}

static int mot_panel_set_backlight(struct udevice *dev, int percent)
{
	struct mot_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	mdelay(5);

	return backlight_set_brightness(priv->backlight, percent);
}

static int mot_panel_timings(struct udevice *dev, struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int mot_panel_of_to_plat(struct udevice *dev)
{
	struct mot_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_debug("%s: cannot get backlight: ret = %d\n", __func__, ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vdd-supply", &priv->vdd);
	if (ret) {
		log_debug("%s: cannot get vdd-supply: ret = %d\n", __func__, ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vddio-supply", &priv->vddio);
	if (ret) {
		log_debug("%s: cannot get vddio-supply: ret = %d\n", __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not decode reser-gpios (%d)\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int mot_panel_hw_init(struct udevice *dev)
{
	struct mot_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->vddio, 1);
	if (ret) {
		log_debug("%s: enabling vddio-supply failed (%d)\n", __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->vdd, 1);
	if (ret) {
		log_debug("%s: enabling vdd-supply failed (%d)\n", __func__, ret);
		return ret;
	}

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_debug("%s: error entering reset (%d)\n", __func__, ret);
		return ret;
	}
	mdelay(50);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_debug("%s: error exiting reset (%d)\n", __func__, ret);
		return ret;
	}
	mdelay(10);

	return 0;
}

static int mot_panel_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_LPM;

	return mot_panel_hw_init(dev);
}

static const struct panel_ops mot_panel_ops = {
	.enable_backlight	= mot_panel_enable_backlight,
	.set_backlight		= mot_panel_set_backlight,
	.get_display_timing	= mot_panel_timings,
};

static const struct udevice_id mot_panel_ids[] = {
	{ .compatible = "motorola,mot-panel" },
	{ }
};

U_BOOT_DRIVER(mot_panel) = {
	.name		= "mot_panel",
	.id		= UCLASS_PANEL,
	.of_match	= mot_panel_ids,
	.ops		= &mot_panel_ops,
	.of_to_plat	= mot_panel_of_to_plat,
	.probe		= mot_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct mot_panel_priv),
};
