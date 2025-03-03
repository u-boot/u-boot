// SPDX-License-Identifier: GPL-2.0+
/*
 * LG LH400WV3-SD04 DSI panel driver
 *
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <asm/gpio.h>

struct lg_lh400wv3_priv {
	struct udevice *avci;
	struct udevice *iovcc;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 29816000,
	.hactive.typ		= 480,
	.hfront_porch.typ	= 10,
	.hback_porch.typ	= 10,
	.hsync_len.typ		= 10,
	.vactive.typ		= 800,
	.vfront_porch.typ	= 4,
	.vback_porch.typ	= 4,
	.vsync_len.typ		= 4,
};

#define dsi_generic_write_seq(dsi, cmd, seq...) do {			\
		static const u8 b[] = { cmd, seq };			\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, b, ARRAY_SIZE(b));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static int lg_lh400wv3_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	dsi_generic_write_seq(dsi, MIPI_DCS_EXIT_INVERT_MODE);
	dsi_generic_write_seq(dsi, MIPI_DCS_SET_TEAR_ON);

	ret = mipi_dsi_dcs_set_pixel_format(dsi, MIPI_DCS_PIXEL_FMT_24BIT |
					    MIPI_DCS_PIXEL_FMT_24BIT << 4);
	if (ret < 0) {
		log_debug("%s: failed to set pixel format: %d\n", __func__, ret);
		return ret;
	}

	dsi_generic_write_seq(dsi, 0xb2, 0x00, 0xc8);
	dsi_generic_write_seq(dsi, 0xb3, 0x00);
	dsi_generic_write_seq(dsi, 0xb4, 0x04);
	dsi_generic_write_seq(dsi, 0xb5, 0x42, 0x10, 0x10, 0x00, 0x20);
	dsi_generic_write_seq(dsi, 0xb6, 0x0b, 0x0f, 0x3c, 0x13, 0x13, 0xe8);
	dsi_generic_write_seq(dsi, 0xb7, 0x4c, 0x06, 0x0c, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0xc0, 0x01, 0x11);
	dsi_generic_write_seq(dsi, 0xc3, 0x07, 0x03, 0x04, 0x04, 0x04);
	dsi_generic_write_seq(dsi, 0xc4, 0x12, 0x24, 0x18, 0x18, 0x02, 0x49);
	dsi_generic_write_seq(dsi, 0xc5, 0x65);
	dsi_generic_write_seq(dsi, 0xc6, 0x41, 0x63);

	dsi_generic_write_seq(dsi, 0xd0, 0x00, 0x46, 0x74, 0x32, 0x1d, 0x03, 0x51, 0x15, 0x04);
	dsi_generic_write_seq(dsi, 0xd1, 0x00, 0x46, 0x74, 0x32, 0x1d, 0x03, 0x51, 0x15, 0x04);
	dsi_generic_write_seq(dsi, 0xd2, 0x00, 0x46, 0x74, 0x32, 0x1f, 0x03, 0x51, 0x15, 0x04);
	dsi_generic_write_seq(dsi, 0xd3, 0x00, 0x46, 0x74, 0x32, 0x1f, 0x03, 0x51, 0x15, 0x04);
	dsi_generic_write_seq(dsi, 0xd4, 0x01, 0x46, 0x74, 0x25, 0x00, 0x03, 0x51, 0x15, 0x04);
	dsi_generic_write_seq(dsi, 0xd5, 0x01, 0x46, 0x74, 0x25, 0x00, 0x03, 0x51, 0x15, 0x04);

	dsi_generic_write_seq(dsi, MIPI_DCS_SET_COLUMN_ADDRESS, 0x00, 0x00, 0x01, 0xdf);
	dsi_generic_write_seq(dsi, MIPI_DCS_SET_PAGE_ADDRESS, 0x00, 0x00, 0x03, 0x1f);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		log_debug("%s: failed to exit sleep mode: %d\n", __func__, ret);
		return ret;
	}

	mdelay(120);

	dsi_generic_write_seq(dsi, MIPI_DCS_WRITE_MEMORY_START);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		log_debug("%s: failed to set display on: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int lg_lh400wv3_set_backlight(struct udevice *dev, int percent)
{
	struct lg_lh400wv3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return backlight_set_brightness(priv->backlight, percent);
}

static int lg_lh400wv3_timings(struct udevice *dev, struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int lg_lh400wv3_of_to_plat(struct udevice *dev)
{
	struct lg_lh400wv3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_debug("%s: cannot get backlight: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "avci-supply", &priv->avci);
	if (ret) {
		log_debug("%s: cannot get avci-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "iovcc-supply", &priv->iovcc);
	if (ret) {
		log_debug("%s: cannot get iovcc-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: cannot decode reset-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int lg_lh400wv3_hw_init(struct udevice *dev)
{
	struct lg_lh400wv3_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_debug("%s: error entering reset (%d)\n", __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->iovcc, 1);
	if (ret) {
		log_debug("%s: enabling iovcc-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->avci, 1);
	if (ret) {
		log_debug("%s: enabling avci-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	mdelay(1);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_debug("%s: error exiting reset (%d)\n", __func__, ret);
		return ret;
	}

	mdelay(10);

	return 0;
}

static int lg_lh400wv3_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM;

	return lg_lh400wv3_hw_init(dev);
}

static const struct panel_ops lg_lh400wv3_ops = {
	.enable_backlight	= lg_lh400wv3_enable_backlight,
	.set_backlight		= lg_lh400wv3_set_backlight,
	.get_display_timing	= lg_lh400wv3_timings,
};

static const struct udevice_id lg_lh400wv3_ids[] = {
	{ .compatible = "lg,lh400wv3-sd04" },
	{ }
};

U_BOOT_DRIVER(lg_lh400wv3) = {
	.name		= "lg_lh400wv3",
	.id		= UCLASS_PANEL,
	.of_match	= lg_lh400wv3_ids,
	.ops		= &lg_lh400wv3_ops,
	.of_to_plat	= lg_lh400wv3_of_to_plat,
	.probe		= lg_lh400wv3_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct lg_lh400wv3_priv),
};
