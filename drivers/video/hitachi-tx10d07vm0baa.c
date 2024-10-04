// SPDX-License-Identifier: GPL-2.0+
/*
 * Hitachi TX10D07VM0BAA DSI panel driver
 *
 * Copyright (c) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <asm/gpio.h>

struct hitachi_tx10d07vm0baa_priv {
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

static int hitachi_tx10d07vm0baa_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	dsi_generic_write_seq(dsi, MIPI_DCS_SET_PARTIAL_AREA, 0x00,
			      0x00, 0x03, 0x1f);
	dsi_generic_write_seq(dsi, MIPI_DCS_SET_SCROLL_AREA, 0x00,
			      0x00, 0x03, 0x20, 0x00, 0x00);

	dsi_generic_write_seq(dsi, MIPI_DCS_SET_ADDRESS_MODE, 0x0a);
	dsi_generic_write_seq(dsi, MIPI_DCS_SET_SCROLL_START, 0x00,
			      0x00);

	ret = mipi_dsi_dcs_set_pixel_format(dsi, MIPI_DCS_PIXEL_FMT_24BIT);
	if (ret) {
		log_debug("%s: failed to set pixel format: %d\n", __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_tear_scanline(dsi, 0x00);
	if (ret) {
		log_debug("%s: failed to set tear scanline: %d\n", __func__, ret);
		return ret;
	}

	dsi_generic_write_seq(dsi, 0x71, 0x00);		/* Ex_Vsync_en */

	dsi_generic_write_seq(dsi, 0xb2, 0x00);		/* VCSEL */
	dsi_generic_write_seq(dsi, 0xb4, 0xaa);		/* setvgmpm */
	dsi_generic_write_seq(dsi, 0xb5, 0x33);		/* rbias1 */
	dsi_generic_write_seq(dsi, 0xb6, 0x03);		/* rbias2 */

	dsi_generic_write_seq(dsi, 0xb7, 0x1a, 0x33, 0x03, 0x03,
			      0x03, 0x00, 0x00, 0x01, 0x02, 0x00,
			      0x00, 0x04, 0x00, 0x01, 0x01, 0x01);	/* set_ddvdhp */
	dsi_generic_write_seq(dsi, 0xb8, 0x1c, 0x53, 0x03, 0x03,
			      0x00, 0x01, 0x02, 0x00, 0x00, 0x04,
			      0x00, 0x01, 0x01);			/* set_ddvdhm */

	dsi_generic_write_seq(dsi, 0xb9, 0x0a, 0x01, 0x01, 0x00,
			      0x00, 0x00, 0x02, 0x00, 0x02, 0x01);	/* set_vgh */
	dsi_generic_write_seq(dsi, 0xba, 0x0f, 0x01, 0x01, 0x00,
			      0x00, 0x00, 0x02, 0x00, 0x02, 0x01);	/* set_vgl */
	dsi_generic_write_seq(dsi, 0xbb, 0x00, 0x00, 0x00, 0x00,
			      0x01, 0x02, 0x01);			/* set_vcl */

	dsi_generic_write_seq(dsi, 0xc1, 0x01);		/* number of lines */
	dsi_generic_write_seq(dsi, 0xc2, 0x08);		/* number of fp lines */
	dsi_generic_write_seq(dsi, 0xc3, 0x04);		/* gateset(1) */
	dsi_generic_write_seq(dsi, 0xc4, 0x4c);		/* 1h period */
	dsi_generic_write_seq(dsi, 0xc5, 0x03);		/* source precharge */
	dsi_generic_write_seq(dsi, 0xc6, 0xc4, 0x04);	/* source precharge timing */
	dsi_generic_write_seq(dsi, 0xc7, 0x00);		/* source level */
	dsi_generic_write_seq(dsi, 0xc8, 0x02);		/* number of bp lines */
	dsi_generic_write_seq(dsi, 0xc9, 0x10);		/* gateset(2) */
	dsi_generic_write_seq(dsi, 0xca, 0x04, 0x04);	/* gateset(3) */
	dsi_generic_write_seq(dsi, 0xcb, 0x03);		/* gateset(4) */
	dsi_generic_write_seq(dsi, 0xcc, 0x12);		/* gateset(5) */
	dsi_generic_write_seq(dsi, 0xcd, 0x12);		/* gateset(6) */
	dsi_generic_write_seq(dsi, 0xce, 0x30);		/* gateset(7) */
	dsi_generic_write_seq(dsi, 0xcf, 0x30);		/* gateset(8) */
	dsi_generic_write_seq(dsi, 0xd0, 0x40);		/* gateset(9) */
	dsi_generic_write_seq(dsi, 0xd1, 0x22);		/* flhw */
	dsi_generic_write_seq(dsi, 0xd2, 0x22);		/* vckhw */
	dsi_generic_write_seq(dsi, 0xd3, 0x04);		/* flt */
	dsi_generic_write_seq(dsi, 0xd4, 0x14);		/* tctrl */
	dsi_generic_write_seq(dsi, 0xd6, 0x02);		/* dotinv */
	dsi_generic_write_seq(dsi, 0xd7, 0x00);		/* on/off sequence period */

	dsi_generic_write_seq(dsi, 0xd8, 0x01, 0x05, 0x06, 0x0d,
			      0x18, 0x09, 0x22, 0x23, 0x00);		/* ponseqa */
	dsi_generic_write_seq(dsi, 0xd9, 0x24, 0x01);			/* ponseqb */
	dsi_generic_write_seq(dsi, 0xde, 0x09, 0x0f, 0x21, 0x12,
			      0x04);					/* ponseqc */

	dsi_generic_write_seq(dsi, 0xdf, 0x02, 0x06, 0x06, 0x06,
			      0x06, 0x00);				/* pofseqa */
	dsi_generic_write_seq(dsi, 0xe0, 0x01);				/* pofseqb */

	ret = mipi_dsi_dcs_set_display_brightness(dsi, 0xff);
	if (ret) {
		log_debug("%s: failed to set display brightness: %d\n", __func__, ret);
		return ret;
	}

	dsi_generic_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x40);

	dsi_generic_write_seq(dsi, 0xe2, 0x00, 0x00);		/* cabc pwm */
	dsi_generic_write_seq(dsi, 0xe3, 0x03);			/* cabc */
	dsi_generic_write_seq(dsi, 0xe4, 0x66, 0x7b, 0x90, 0xa5,
			      0xbb, 0xc7, 0xe1, 0xe5);		/* cabc brightness */
	dsi_generic_write_seq(dsi, 0xe5, 0xc5, 0xc5, 0xc9, 0xc9,
			      0xd1, 0xe1, 0xf1, 0xfe);		/* cabc brightness */
	dsi_generic_write_seq(dsi, 0xe7, 0x2a);			/* cabc */
	dsi_generic_write_seq(dsi, 0xe8, 0x00);			/* brt_rev */
	dsi_generic_write_seq(dsi, 0xe9, 0x00);			/* tefreq */

	dsi_generic_write_seq(dsi, 0xea, 0x01);			/* high speed ram */

	dsi_generic_write_seq(dsi, 0xeb, 0x00, 0x33, 0x0e, 0x15,
			      0xb7, 0x78, 0x88, 0x0f);		/* gamma setting r pos */
	dsi_generic_write_seq(dsi, 0xec, 0x00, 0x33, 0x0e, 0x15,
			      0xb7, 0x78, 0x88, 0x0f);		/* gamma setting r neg */
	dsi_generic_write_seq(dsi, 0xed, 0x00, 0x33, 0x0e, 0x15,
			      0xb7, 0x78, 0x88, 0x0f);		/* gamma setting g pos */
	dsi_generic_write_seq(dsi, 0xee, 0x00, 0x33, 0x0e, 0x15,
			      0xb7, 0x78, 0x88, 0x0f);		/* gamma setting g neg */
	dsi_generic_write_seq(dsi, 0xef, 0x00, 0x33, 0x0e, 0x15,
			      0xb7, 0x78, 0x88, 0x0f);		/* gamma setting b pos */
	dsi_generic_write_seq(dsi, 0xf0, 0x00, 0x33, 0x0e, 0x15,
			      0xb7, 0x78, 0x88, 0x0f);		/* gamma setting b neg */

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret) {
		log_debug("%s: failed to exit sleep mode: %d\n", __func__, ret);
		return ret;
	}

	mdelay(110);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret) {
		log_debug("%s: failed to set display on: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int hitachi_tx10d07vm0baa_set_backlight(struct udevice *dev, int percent)
{
	struct hitachi_tx10d07vm0baa_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return backlight_set_brightness(priv->backlight, percent);
}

static int hitachi_tx10d07vm0baa_timings(struct udevice *dev,
					 struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int hitachi_tx10d07vm0baa_of_to_plat(struct udevice *dev)
{
	struct hitachi_tx10d07vm0baa_priv *priv = dev_get_priv(dev);
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

static int hitachi_tx10d07vm0baa_hw_init(struct udevice *dev)
{
	struct hitachi_tx10d07vm0baa_priv *priv = dev_get_priv(dev);
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

	mdelay(25);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_debug("%s: error exiting reset (%d)\n", __func__, ret);
		return ret;
	}

	mdelay(5);

	return 0;
}

static int hitachi_tx10d07vm0baa_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM;

	return hitachi_tx10d07vm0baa_hw_init(dev);
}

static const struct panel_ops hitachi_tx10d07vm0baa_ops = {
	.enable_backlight	= hitachi_tx10d07vm0baa_enable_backlight,
	.set_backlight		= hitachi_tx10d07vm0baa_set_backlight,
	.get_display_timing	= hitachi_tx10d07vm0baa_timings,
};

static const struct udevice_id hitachi_tx10d07vm0baa_ids[] = {
	{ .compatible = "hit,tx10d07vm0baa" },
	{ }
};

U_BOOT_DRIVER(hitachi_tx10d07vm0baa) = {
	.name		= "hitachi_tx10d07vm0baa",
	.id		= UCLASS_PANEL,
	.of_match	= hitachi_tx10d07vm0baa_ids,
	.ops		= &hitachi_tx10d07vm0baa_ops,
	.of_to_plat	= hitachi_tx10d07vm0baa_of_to_plat,
	.probe		= hitachi_tx10d07vm0baa_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct hitachi_tx10d07vm0baa_priv),
};
