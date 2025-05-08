// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <backlight.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <linux/err.h>
#include <asm/gpio.h>
#include <power/regulator.h>

#define AAT2870_BL_MIN_BRIGHTNESS	0x01
#define AAT2870_BL_DEF_BRIGHTNESS	0x64
#define AAT2870_BL_MAX_BRIGHTNESS	0xff

#define AAT2870_BL_CH_EN		0x00
#define AAT2870_BLM			0x01

#define AAT2870_BL_CH_ALL		0xff

#define AAT2870_CURRENT_MAX		27900000
#define AAT2870_CURRENT_STEP		900000

struct aat2870_backlight_priv {
	struct gpio_desc enable_gpio;

	int channels;
	int max_current;
};

static int aat2870_backlight_enable(struct udevice *dev)
{
	struct aat2870_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	dm_gpio_set_value(&priv->enable_gpio, 1);

	/* Enable backlight for defined set of channels */
	ret = dm_i2c_reg_write(dev, AAT2870_BL_CH_EN, priv->channels);
	if (ret)
		return ret;

	return 0;
}

static int aat2870_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct aat2870_backlight_priv *priv = dev_get_priv(dev);
	int brightness, ret;

	if (percent == BACKLIGHT_DEFAULT)
		percent = AAT2870_BL_DEF_BRIGHTNESS;

	if (percent < AAT2870_BL_MIN_BRIGHTNESS)
		percent = AAT2870_BL_MIN_BRIGHTNESS;

	if (percent > AAT2870_BL_MAX_BRIGHTNESS)
		percent = AAT2870_BL_MAX_BRIGHTNESS;

	brightness = percent * priv->max_current;
	brightness /= AAT2870_BL_MAX_BRIGHTNESS;

	/* Set brightness level */
	ret = dm_i2c_reg_write(dev, AAT2870_BLM, brightness);
	if (ret)
		return ret;

	return 0;
}

static int aat2870_backlight_of_to_plat(struct udevice *dev)
{
	struct aat2870_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_err("%s: cannot get enable-gpios (%d)\n",
			__func__, ret);
		return ret;
	}

	/* Backlight is one of children but has no dedicated driver */
	ofnode backlight = ofnode_find_subnode(dev_ofnode(dev), "backlight");
	if (ofnode_valid(backlight) && ofnode_is_enabled(backlight)) {
		/* Number of channel is equal to bit number */
		priv->channels = dev_read_u32_default(dev, "channels", AAT2870_BL_CH_ALL);
		if (priv->channels != AAT2870_BL_CH_ALL)
			priv->channels = BIT(priv->channels);

		/* 450mA - 27900mA range with a 900mA step */
		priv->max_current = dev_read_u32_default(dev, "current-max-microamp",
							 AAT2870_CURRENT_MAX);
		priv->max_current /= AAT2870_CURRENT_STEP;
	}

	return 0;
}

static int aat2870_backlight_probe(struct udevice *dev)
{
	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	return 0;
}

static const struct backlight_ops aat2870_backlight_ops = {
	.enable = aat2870_backlight_enable,
	.set_brightness = aat2870_backlight_set_brightness,
};

static const struct udevice_id aat2870_backlight_ids[] = {
	{ .compatible = "analogictech,aat2870" },
	{ .compatible = "skyworks,aat2870" },
	{ }
};

U_BOOT_DRIVER(aat2870_backlight) = {
	.name		= "aat2870_backlight",
	.id		= UCLASS_PANEL_BACKLIGHT,
	.of_match	= aat2870_backlight_ids,
	.of_to_plat	= aat2870_backlight_of_to_plat,
	.probe		= aat2870_backlight_probe,
	.ops		= &aat2870_backlight_ops,
	.priv_auto	= sizeof(struct aat2870_backlight_priv),
};
