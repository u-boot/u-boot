// SPDX-License-Identifier: GPL-2.0+
/*
 * Sharp LQ079L1SX01 DSI panel driver
 *
 * Copyright (c) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <power/regulator.h>

struct sharp_lq079l1sx01_priv {
	struct udevice *backlight;
	struct udevice *panel_sec;

	struct udevice *avdd;
	struct udevice *vddio;
	struct udevice *vsp;
	struct udevice *vsn;

	struct gpio_desc reset_gpio;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= 215000000,
	.hactive.typ		= 1536,
	.hfront_porch.typ	= 136,
	.hback_porch.typ	= 28,
	.hsync_len.typ		= 28,
	.vactive.typ		= 2048,
	.vfront_porch.typ	= 14,
	.vback_porch.typ	= 8,
	.vsync_len.typ		= 2,
};

static int dcs_write_one(struct mipi_dsi_device *dsi, u8 cmd, u8 data)
{
	return mipi_dsi_dcs_write(dsi, cmd, &data, 1);
}

static int sharp_lq079l1sx01_configure_link(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		log_debug("%s: failed to exit sleep mode %s: %d\n",
			  __func__, dev->parent->name, ret);
	}
	mdelay(120);

	ret = dcs_write_one(dsi, MIPI_DCS_SET_DISPLAY_BRIGHTNESS, 0xff);
	if (ret < 0) {
		log_debug("%s: failed to SET_DISPLAY_BRIGHTNESS %s: %d\n",
			  __func__, dev->parent->name, ret);
	}
	ret = dcs_write_one(dsi, MIPI_DCS_WRITE_POWER_SAVE, 0x01);
	if (ret < 0) {
		log_debug("%s: failed to WRITE_POWER_SAVE %s: %d\n",
			  __func__, dev->parent->name, ret);
	}
	ret = dcs_write_one(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x2c);
	if (ret < 0) {
		log_debug("%s: failed to WRITE_CONTROL_DISPLAY %s: %d\n",
			  __func__, dev->parent->name, ret);
	}

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		log_debug("%s: failed to set panel on %s: %d\n",
			  __func__, dev->parent->name, ret);
	}

	return 0;
}

static int sharp_lq079l1sx01_enable_backlight(struct udevice *dev)
{
	struct sharp_lq079l1sx01_priv *priv = dev_get_priv(dev);

	if (!priv->panel_sec)
		return 0;

	sharp_lq079l1sx01_configure_link(dev);
	sharp_lq079l1sx01_configure_link(priv->panel_sec);

	return 0;
}

static int sharp_lq079l1sx01_set_backlight(struct udevice *dev, int percent)
{
	struct sharp_lq079l1sx01_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv->panel_sec)
		return 0;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;

	return backlight_set_brightness(priv->backlight, percent);
}

static int sharp_lq079l1sx01_timings(struct udevice *dev,
				     struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int sharp_lq079l1sx01_of_to_plat(struct udevice *dev)
{
	struct sharp_lq079l1sx01_priv *priv = dev_get_priv(dev);
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
					   "avdd-supply", &priv->avdd);
	if (ret) {
		log_debug("%s: cannot get avdd-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vddio-supply", &priv->vddio);
	if (ret) {
		log_debug("%s: cannot get vddio-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vsp-supply", &priv->vsp);
	if (ret) {
		log_debug("%s: cannot get vsp-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "vsn-supply", &priv->vsn);
	if (ret) {
		log_debug("%s: cannot get vsn-supply: ret = %d\n",
			  __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: cannot get reser-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int sharp_lq079l1sx01_hw_init(struct udevice *dev)
{
	struct sharp_lq079l1sx01_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv->panel_sec)
		return 0;

	ret = regulator_set_enable_if_allowed(priv->vddio, 1);
	if (ret) {
		log_debug("%s: enabling vddio-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->avdd, 1);
	if (ret) {
		log_debug("%s: enabling avdd-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	mdelay(12);

	ret = regulator_set_enable_if_allowed(priv->vsp, 1);
	if (ret) {
		log_debug("%s: enabling vsp-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	mdelay(12);

	ret = regulator_set_enable_if_allowed(priv->vsn, 1);
	if (ret) {
		log_debug("%s: enabling vsn-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	mdelay(24);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_debug("%s: error disabling reset-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}
	udelay(3);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret) {
		log_debug("%s: error enabling reset-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}
	udelay(3);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret) {
		log_debug("%s: error disabling reset-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}
	mdelay(32);

	return 0;
}

static int sharp_lq079l1sx01_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	/* fill characteristics of DSI data link */
	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM;

	return sharp_lq079l1sx01_hw_init(dev);
}

static const struct panel_ops sharp_lq079l1sx01_ops = {
	.enable_backlight	= sharp_lq079l1sx01_enable_backlight,
	.set_backlight		= sharp_lq079l1sx01_set_backlight,
	.get_display_timing	= sharp_lq079l1sx01_timings,
};

static const struct udevice_id sharp_lq079l1sx01_ids[] = {
	{ .compatible = "sharp,lq079l1sx01" },
	{ }
};

U_BOOT_DRIVER(sharp_lq079l1sx01) = {
	.name		= "sharp_lq079l1sx01",
	.id		= UCLASS_PANEL,
	.of_match	= sharp_lq079l1sx01_ids,
	.ops		= &sharp_lq079l1sx01_ops,
	.of_to_plat	= sharp_lq079l1sx01_of_to_plat,
	.probe		= sharp_lq079l1sx01_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct sharp_lq079l1sx01_priv),
};
