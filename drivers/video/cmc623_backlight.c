// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Ion Agorria <ion@agorria.com>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <backlight.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <linux/err.h>
#include <asm/gpio.h>
#include <power/regulator.h>

#define CMC623_I2C_REG_SELBANK		0x00
#define CMC623_I2C_REG_PWMCTRL		0xb4
#define CMC623_I2C_REG_REGMASK		0x28

#define CMC623_BL_MIN_BRIGHTNESS	0
#define CMC623_BL_DEF_BRIGHTNESS	50
#define CMC623_BL_MAX_BRIGHTNESS	100

struct cmc623_backlight_priv {
	struct gpio_desc enable_gpio;
};

static int cmc623_backlight_enable(struct udevice *dev)
{
	struct cmc623_backlight_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->enable_gpio))
		dm_gpio_set_value(&priv->enable_gpio, 1);

	return 0;
}

static int cmc623_i2c_write(struct udevice *dev, u8 reg, u16 value)
{
	u8 data[2];

	data[0] = (value >> 8) & 0xff;
	data[1] = value & 0xff;

	return dm_i2c_write(dev->parent, reg, data, 2);
}

static int cmc623_backlight_set_brightness(struct udevice *dev, int percent)
{
	int ret;
	u16 brightness;

	if (percent == BACKLIGHT_DEFAULT)
		percent = CMC623_BL_DEF_BRIGHTNESS;

	if (percent < CMC623_BL_MIN_BRIGHTNESS)
		percent = CMC623_BL_MIN_BRIGHTNESS;

	if (percent > CMC623_BL_MAX_BRIGHTNESS)
		percent = CMC623_BL_MAX_BRIGHTNESS;

	brightness = 0x4000 | (percent << 4);

	ret = cmc623_i2c_write(dev, CMC623_I2C_REG_SELBANK, 0x0000);
	if (ret) {
		log_debug("%s: error at CMC623_I2C_REG_SELBANK (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = cmc623_i2c_write(dev, CMC623_I2C_REG_PWMCTRL, brightness);
	if (ret) {
		log_debug("%s: error at CMC623_I2C_REG_PWMCTRL (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = cmc623_i2c_write(dev, CMC623_I2C_REG_REGMASK, 0x0000);
	if (ret) {
		log_debug("%s: error at CMC623_I2C_REG_REGMASK (%d)\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int cmc623_backlight_of_to_plat(struct udevice *dev)
{
	struct cmc623_backlight_priv *priv = dev_get_priv(dev);

	gpio_request_by_name(dev, "enable-gpios", 0,
			     &priv->enable_gpio, GPIOD_IS_OUT);

	return 0;
}

static int cmc623_backlight_probe(struct udevice *dev)
{
	if (device_get_uclass_id(dev->parent) != UCLASS_VIDEO_BRIDGE)
		return -EPROTONOSUPPORT;

	return 0;
}

static const struct backlight_ops cmc623_backlight_ops = {
	.enable = cmc623_backlight_enable,
	.set_brightness = cmc623_backlight_set_brightness,
};

static const struct udevice_id cmc623_backlight_ids[] = {
	{ .compatible = "samsung,cmc623-backlight" },
	{ }
};

U_BOOT_DRIVER(cmc623_backlight) = {
	.name		= "cmc623_backlight",
	.id		= UCLASS_PANEL_BACKLIGHT,
	.of_match	= cmc623_backlight_ids,
	.of_to_plat	= cmc623_backlight_of_to_plat,
	.probe		= cmc623_backlight_probe,
	.ops		= &cmc623_backlight_ops,
	.priv_auto	= sizeof(struct cmc623_backlight_priv),
};
