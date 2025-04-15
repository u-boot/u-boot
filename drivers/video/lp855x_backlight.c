// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Texas Instruments
 * Copyright (c) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <malloc.h>
#include <backlight.h>
#include <dm.h>
#include <dm/devres.h>
#include <i2c.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/gpio.h>
#include <power/regulator.h>

#define LP855x_MIN_BRIGHTNESS		0x00
#define LP855x_MAX_BRIGHTNESS		0xff

/* LP8550/1/2/3/6 Registers */
#define LP855X_BRIGHTNESS_CTRL		0x00
#define LP855X_DEVICE_CTRL		0x01
#define LP855X_EEPROM_START		0xa0
#define LP855X_EEPROM_END		0xa7
#define LP8556_EPROM_START		0x98
#define LP8556_EPROM_END		0xaf

/* LP8555/7 Registers */
#define LP8557_BL_CMD			0x00
#define LP8557_BL_MASK			0x01
#define LP8557_BL_ON			0x01
#define LP8557_BL_OFF			0x00
#define LP8557_BRIGHTNESS_CTRL		0x04
#define LP8557_CONFIG			0x10
#define LP8555_EPROM_START		0x10
#define LP8555_EPROM_END		0x7a
#define LP8557_EPROM_START		0x10
#define LP8557_EPROM_END		0x1e

struct lp855x_rom_data {
	u8 addr;
	u8 val;
};

struct lp855x_backlight_priv;

/*
 * struct lp855x_device_config
 * @pre_init_device: init device function call before updating the brightness
 * @reg_brightness: register address for brigthenss control
 * @reg_devicectrl: register address for device control
 * @post_init_device: late init device function call
 */
struct lp855x_device_config {
	int (*pre_init_device)(struct udevice *dev);
	u8 reg_brightness;
	u8 reg_devicectrl;
	u8 reg_eepromstart;
	u8 reg_eepromend;
	int (*post_init_device)(struct udevice *dev);
};

struct lp855x_backlight_priv {
	struct udevice *supply;	/* regulator for VDD input */
	struct udevice *enable;	/* regulator for EN/VDDIO input */

	u8 device_control;
	u8 initial_brightness;

	int size_program;
	struct lp855x_rom_data *rom_data;
	struct lp855x_device_config *cfg;
};

static int lp855x_backlight_enable(struct udevice *dev)
{
	struct lp855x_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->supply, 1);
	if (ret) {
		log_debug("%s: enabling power-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->enable, 1);
	if (ret) {
		log_debug("%s: enabling enable-supply failed (%d)\n",
			  __func__, ret);
		return ret;
	}
	mdelay(2);

	if (priv->cfg->pre_init_device) {
		ret = priv->cfg->pre_init_device(dev);
		if (ret) {
			log_debug("%s: pre init device err: %d\n",
				  __func__, ret);
			return ret;
		}
	}

	ret = dm_i2c_reg_write(dev, priv->cfg->reg_brightness,
			       priv->initial_brightness);
	if (ret)
		return ret;

	ret = dm_i2c_reg_write(dev, priv->cfg->reg_devicectrl,
			       priv->device_control);
	if (ret)
		return ret;

	if (priv->size_program > 0) {
		int i;
		u8 val, addr;

		for (i = 0; i < priv->size_program; i++) {
			addr = priv->rom_data[i].addr;
			val = priv->rom_data[i].val;

			if (addr < priv->cfg->reg_eepromstart &&
			    addr > priv->cfg->reg_eepromend)
				continue;

			ret = dm_i2c_reg_write(dev, addr, val);
			if (ret)
				return ret;
		}
	}

	if (priv->cfg->post_init_device) {
		ret = priv->cfg->post_init_device(dev);
		if (ret) {
			log_debug("%s: post init device err: %d\n",
				  __func__, ret);
			return ret;
		}
	}

	return 0;
}

static int lp855x_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct lp855x_backlight_priv *priv = dev_get_priv(dev);

	if (percent == BACKLIGHT_DEFAULT)
		percent = priv->initial_brightness;

	if (percent < LP855x_MIN_BRIGHTNESS)
		percent = LP855x_MIN_BRIGHTNESS;

	if (percent > LP855x_MAX_BRIGHTNESS)
		percent = LP855x_MAX_BRIGHTNESS;

	/* Set brightness level */
	return dm_i2c_reg_write(dev, priv->cfg->reg_brightness,
				percent);
}

static int lp855x_backlight_probe(struct udevice *dev)
{
	struct lp855x_backlight_priv *priv = dev_get_priv(dev);
	int rom_length, ret;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	priv->cfg = (struct lp855x_device_config *)dev_get_driver_data(dev);

	dev_read_u8(dev, "dev-ctrl", &priv->device_control);
	dev_read_u8(dev, "init-brt", &priv->initial_brightness);

	/* Fill ROM platform data if defined */
	rom_length = dev_get_child_count(dev);
	if (rom_length > 0) {
		struct lp855x_rom_data *rom;
		ofnode child;
		int i = 0;

		rom = devm_kcalloc(dev, rom_length, sizeof(*rom), GFP_KERNEL);
		if (!rom)
			return -ENOMEM;

		dev_for_each_subnode(child, dev) {
			ofnode_read_u8(child, "rom-addr", &rom[i].addr);
			ofnode_read_u8(child, "rom-val", &rom[i].val);
			i++;
		}

		priv->size_program = rom_length;
		priv->rom_data = &rom[0];
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "power-supply", &priv->supply);
	if (ret) {
		log_err("%s: cannot get power-supply: ret = %d\n", __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "enable-supply", &priv->enable);
	if (ret) {
		log_err("%s: cannot get enable-supply: ret = %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static const struct backlight_ops lp855x_backlight_ops = {
	.enable = lp855x_backlight_enable,
	.set_brightness = lp855x_backlight_set_brightness,
};

static int lp8556_bl_rst(struct udevice *dev)
{
	int ret;

	/* Reset backlight after updating EPROM settings */
	ret = dm_i2c_reg_clrset(dev, LP855X_DEVICE_CTRL, LP8557_BL_MASK,
				LP8557_BL_OFF);
	if (ret)
		return ret;

	mdelay(10);

	return dm_i2c_reg_clrset(dev, LP855X_DEVICE_CTRL, LP8557_BL_MASK,
				 LP8557_BL_ON);
}

static int lp8557_bl_off(struct udevice *dev)
{
	/* BL_ON = 0 before updating EPROM settings */
	return dm_i2c_reg_clrset(dev, LP8557_BL_CMD, LP8557_BL_MASK,
				 LP8557_BL_OFF);
}

static int lp8557_bl_on(struct udevice *dev)
{
	/* BL_ON = 1 after updating EPROM settings */
	return dm_i2c_reg_clrset(dev, LP8557_BL_CMD, LP8557_BL_MASK,
				 LP8557_BL_ON);
}

static struct lp855x_device_config lp855x_dev_cfg = {
	.reg_brightness = LP855X_BRIGHTNESS_CTRL,
	.reg_devicectrl = LP855X_DEVICE_CTRL,
	.reg_eepromstart = LP855X_EEPROM_START,
	.reg_eepromend = LP855X_EEPROM_END,
};

static struct lp855x_device_config lp8555_dev_cfg = {
	.reg_brightness = LP8557_BRIGHTNESS_CTRL,
	.reg_devicectrl = LP8557_CONFIG,
	.reg_eepromstart = LP8555_EPROM_START,
	.reg_eepromend = LP8555_EPROM_END,
	.pre_init_device = lp8557_bl_off,
	.post_init_device = lp8557_bl_on,
};

static struct lp855x_device_config lp8556_dev_cfg = {
	.reg_brightness = LP855X_BRIGHTNESS_CTRL,
	.reg_devicectrl = LP855X_DEVICE_CTRL,
	.reg_eepromstart = LP8556_EPROM_START,
	.reg_eepromend = LP8556_EPROM_END,
	.post_init_device = lp8556_bl_rst,
};

static struct lp855x_device_config lp8557_dev_cfg = {
	.reg_brightness = LP8557_BRIGHTNESS_CTRL,
	.reg_devicectrl = LP8557_CONFIG,
	.reg_eepromstart = LP8557_EPROM_START,
	.reg_eepromend = LP8557_EPROM_END,
	.pre_init_device = lp8557_bl_off,
	.post_init_device = lp8557_bl_on,
};

static const struct udevice_id lp855x_backlight_ids[] = {
	{ .compatible = "ti,lp8550", .data = (ulong)&lp855x_dev_cfg },
	{ .compatible = "ti,lp8551", .data = (ulong)&lp855x_dev_cfg },
	{ .compatible = "ti,lp8552", .data = (ulong)&lp855x_dev_cfg },
	{ .compatible = "ti,lp8553", .data = (ulong)&lp855x_dev_cfg },
	{ .compatible = "ti,lp8555", .data = (ulong)&lp8555_dev_cfg },
	{ .compatible = "ti,lp8556", .data = (ulong)&lp8556_dev_cfg },
	{ .compatible = "ti,lp8557", .data = (ulong)&lp8557_dev_cfg },
	{ }
};

U_BOOT_DRIVER(lp855x_backlight) = {
	.name		= "lp855x_backlight",
	.id		= UCLASS_PANEL_BACKLIGHT,
	.of_match	= lp855x_backlight_ids,
	.probe		= lp855x_backlight_probe,
	.ops		= &lp855x_backlight_ops,
	.priv_auto	= sizeof(struct lp855x_backlight_priv),
};
