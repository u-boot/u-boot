// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <backlight.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <i2c.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/gpio.h>

#define LM3533_BL_MIN_BRIGHTNESS			0x02
#define LM3533_BL_MAX_BRIGHTNESS			0xFF

#define LM3533_SINK_OUTPUT_CONFIG_1			0x10
#define LM3533_CONTROL_PWM_BASE				0x14
#define   PWM_MAX					GENMASK(5, 0)
#define LM3533_CONTROL_BANK_AB_BRIGHTNESS		0x1A
#define LM3533_CONTROL_FULLSCALE_CURRENT_BASE		0x1F
#define   MAX_CURRENT_MIN				5000
#define   MAX_CURRENT_MAX				29800
#define   MAX_CURRENT_STEP				800
#define LM3533_CONTROL_BANK_ENABLE			0x27
#define LM3533_OVP_FREQUENCY_PWM_POLARITY		0x2C
#define   BOOST_OVP_MASK				GENMASK(2, 1)
#define   BOOST_OVP_SHIFT				1
#define   BOOST_FREQ_MASK				BIT(0)
#define   BOOST_FREQ_SHIFT				0
#define LM3533_BRIGHTNESS_REGISTER_A			0x40

#define LM3533_BOOST_OVP_16V				16000000UL
#define LM3533_BOOST_FREQ_500KHZ			500000UL

struct lm3533_backlight_priv {
	struct gpio_desc enable_gpio;
	u32 def_bl_lvl;

	/* Core */
	u32 boost_ovp;
	u32 boost_freq;

	/* Backlight */
	u32 reg;
	u16 max_current;		/* 5000 - 29800 uA (800 uA step) */
	u8 pwm;				/* 0 - 0x3f */
	bool linear;
	bool hvled;
};

static int lm3533_backlight_enable(struct udevice *dev)
{
	struct lm3533_backlight_priv *priv = dev_get_priv(dev);
	u8 val, id = priv->reg;
	int ret;

	if (priv->linear) {
		ret = dm_i2c_reg_clrset(dev, LM3533_CONTROL_BANK_AB_BRIGHTNESS,
					BIT(2 * id + 1), BIT(2 * id + 1));
		if (ret)
			return ret;
	}

	if (priv->hvled) {
		ret = dm_i2c_reg_clrset(dev, LM3533_SINK_OUTPUT_CONFIG_1,
					BIT(0) | BIT(1), id | id << 1);
		if (ret)
			return ret;
	}

	/* Set current */
	if (priv->max_current < MAX_CURRENT_MIN || priv->max_current > MAX_CURRENT_MAX)
		return -EINVAL;

	val = (priv->max_current - MAX_CURRENT_MIN) / MAX_CURRENT_STEP;
	ret = dm_i2c_reg_write(dev, LM3533_CONTROL_FULLSCALE_CURRENT_BASE + id, val);
	if (ret)
		return ret;

	/* Set PWM mask */
	if (priv->pwm > PWM_MAX)
		return -EINVAL;

	ret = dm_i2c_reg_write(dev, LM3533_CONTROL_PWM_BASE + id, priv->pwm);
	if (ret)
		return ret;

	/* Enable Control Bank */
	return dm_i2c_reg_clrset(dev, LM3533_CONTROL_BANK_ENABLE, BIT(id), BIT(id));
}

static int lm3533_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct lm3533_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	if (percent == BACKLIGHT_DEFAULT)
		percent = priv->def_bl_lvl;

	if (percent < LM3533_BL_MIN_BRIGHTNESS)
		percent = LM3533_BL_MIN_BRIGHTNESS;

	if (percent > LM3533_BL_MAX_BRIGHTNESS)
		percent = LM3533_BL_MAX_BRIGHTNESS;

	/* Set brightness level */
	ret = dm_i2c_reg_write(dev, LM3533_BRIGHTNESS_REGISTER_A,
			       percent);
	if (ret)
		return ret;

	return 0;
}

static int lm3533_backlight_of_to_plat(struct udevice *dev)
{
	struct lm3533_backlight_priv *priv = dev_get_priv(dev);
	ofnode child;
	int ret;

	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_err("Could not decode enable-gpios (%d)\n", ret);
		return ret;
	}

	priv->boost_ovp = dev_read_u32_default(dev, "ti,boost-ovp-microvolt",
					       LM3533_BOOST_OVP_16V);

	/* boost_ovp is defined in microvolts, convert to enum value */
	priv->boost_ovp = priv->boost_ovp / (8 * 1000 * 1000) - 2;

	priv->boost_freq = dev_read_u32_default(dev, "ti,boost-freq-hz",
						LM3533_BOOST_FREQ_500KHZ);

	/* boost_freq is defined in Hz, convert to enum value */
	priv->boost_freq = priv->boost_freq / (500 * 1000) - 1;

	/* Backlight is one of children but has no dedicated driver */
	ofnode_for_each_subnode(child, dev_ofnode(dev)) {
		if (ofnode_device_is_compatible(child, "ti,lm3533-backlight")) {
			const char *node_name = ofnode_get_name(child);

			if (!strcmp(&node_name[10], "1"))
				priv->reg = 1;
			else
				priv->reg = 0;

			priv->max_current = ofnode_read_u32_default(child, "ti,max-current-microamp",
								    5000);
			priv->pwm = ofnode_read_u32_default(child, "ti,pwm-config-mask", 0);

			priv->def_bl_lvl = ofnode_read_u32_default(child, "default-brightness",
								   LM3533_BL_MAX_BRIGHTNESS);

			priv->linear = ofnode_read_bool(child, "ti,linear-mapping-mode");
			priv->hvled = ofnode_read_bool(child, "ti,hardware-controlled");
		}
	}

	return 0;
}

static int lm3533_backlight_probe(struct udevice *dev)
{
	struct lm3533_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	dm_gpio_set_value(&priv->enable_gpio, 1);
	mdelay(5);

	ret = dm_i2c_reg_clrset(dev, LM3533_OVP_FREQUENCY_PWM_POLARITY,
				BOOST_FREQ_MASK, priv->boost_freq << BOOST_FREQ_SHIFT);
	if (ret) {
		log_debug("%s: freq config failed %d\n", __func__, ret);
		return ret;
	}

	ret = dm_i2c_reg_clrset(dev, LM3533_OVP_FREQUENCY_PWM_POLARITY,
				BOOST_OVP_MASK, priv->boost_ovp << BOOST_OVP_SHIFT);
	if (ret) {
		log_debug("%s: ovp config failed %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static const struct backlight_ops lm3533_backlight_ops = {
	.enable = lm3533_backlight_enable,
	.set_brightness = lm3533_backlight_set_brightness,
};

static const struct udevice_id lm3533_backlight_ids[] = {
	{ .compatible = "ti,lm3533" },
	{ }
};

U_BOOT_DRIVER(lm3533_backlight) = {
	.name		= "lm3533_backlight",
	.id		= UCLASS_PANEL_BACKLIGHT,
	.of_match	= lm3533_backlight_ids,
	.of_to_plat	= lm3533_backlight_of_to_plat,
	.probe		= lm3533_backlight_probe,
	.ops		= &lm3533_backlight_ops,
	.priv_auto	= sizeof(struct lm3533_backlight_priv),
};
