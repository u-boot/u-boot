// SPDX-License-Identifier: GPL-2.0+
/*
 * TI LM3532 LED driver
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
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
#include <power/regulator.h>

#define LM3532_BL_MODE_MANUAL		0x00
#define LM3532_BL_MODE_ALS		0x01

#define LM3532_REG_OUTPUT_CFG		0x10
#define LM3532_REG_STARTSHUT_RAMP	0x11
#define LM3532_REG_RT_RAMP		0x12
#define LM3532_REG_PWM_A_CFG		0x13
#define LM3532_REG_PWM_B_CFG		0x14
#define LM3532_REG_PWM_C_CFG		0x15
#define LM3532_REG_ZONE_CFG_A		0x16
#define LM3532_REG_CTRL_A_FS_CURR	0x17
#define LM3532_REG_ZONE_CFG_B		0x18
#define LM3532_REG_CTRL_B_FS_CURR	0x19
#define LM3532_REG_ZONE_CFG_C		0x1a
#define LM3532_REG_CTRL_C_FS_CURR	0x1b
#define LM3532_REG_ENABLE		0x1d
#define LM3532_ALS_CONFIG		0x23
#define LM3532_REG_ZN_0_HI		0x60
#define LM3532_REG_ZN_0_LO		0x61
#define LM3532_REG_ZN_1_HI		0x62
#define LM3532_REG_ZN_1_LO		0x63
#define LM3532_REG_ZN_2_HI		0x64
#define LM3532_REG_ZN_2_LO		0x65
#define LM3532_REG_ZN_3_HI		0x66
#define LM3532_REG_ZN_3_LO		0x67
#define LM3532_REG_ZONE_TRGT_A		0x70
#define LM3532_REG_ZONE_TRGT_B		0x75
#define LM3532_REG_ZONE_TRGT_C		0x7a
#define LM3532_REG_MAX			0x7e

/* Control Enable */
#define LM3532_CTRL_A_ENABLE		BIT(0)
#define LM3532_CTRL_B_ENABLE		BIT(1)
#define LM3532_CTRL_C_ENABLE		BIT(2)

/* PWM Zone Control */
#define LM3532_PWM_ZONE_MASK		0x7c
#define LM3532_PWM_ZONE_0_EN		BIT(2)
#define LM3532_PWM_ZONE_1_EN		BIT(3)
#define LM3532_PWM_ZONE_2_EN		BIT(4)
#define LM3532_PWM_ZONE_3_EN		BIT(5)
#define LM3532_PWM_ZONE_4_EN		BIT(6)

/* Brightness Configuration */
#define LM3532_I2C_CTRL			BIT(0)
#define LM3532_ALS_CTRL			0
#define LM3532_LINEAR_MAP		BIT(1)
#define LM3532_ZONE_MASK		(BIT(2) | BIT(3) | BIT(4))
#define LM3532_ZONE_0			0
#define LM3532_ZONE_1			BIT(2)
#define LM3532_ZONE_2			BIT(3)
#define LM3532_ZONE_3			(BIT(2) | BIT(3))
#define LM3532_ZONE_4			BIT(4)

#define LM3532_ENABLE_ALS		BIT(3)
#define LM3532_ALS_SEL_SHIFT		6

/* Zone Boundary Register */
#define LM3532_ALS_WINDOW_mV		2000
#define LM3532_ALS_ZB_MAX		4
#define LM3532_ALS_OFFSET_mV		2

#define LM3532_CONTROL_A		0
#define LM3532_CONTROL_B		1
#define LM3532_CONTROL_C		2
#define LM3532_MAX_CONTROL_BANKS	3
#define LM3532_MAX_LED_STRINGS		3

#define LM3532_OUTPUT_CFG_MASK		0x3
#define LM3532_BRT_VAL_ADJUST		8
#define LM3532_RAMP_DOWN_SHIFT		3

#define LM3532_NUM_RAMP_VALS		8
#define LM3532_NUM_AVG_VALS		8
#define LM3532_NUM_IMP_VALS		32

#define LM3532_FS_CURR_MIN		5000
#define LM3532_FS_CURR_MAX		29800
#define LM3532_FS_CURR_STEP		800

struct lm3532_bank_data {
	int control_bank;
	int mode;
	int ctrl_brt_pointer;
	int num_leds;
	int full_scale_current;
	u32 present:1;
	u32 led_strings[LM3532_MAX_CONTROL_BANKS];
};

struct lm3532_backlight_priv {
	struct gpio_desc enable_gpio;
	struct udevice *regulator;

	u32 runtime_ramp_up;
	u32 runtime_ramp_down;

	struct lm3532_bank_data bank[LM3532_MAX_CONTROL_BANKS];
};

/* This device does not like i2c md so use this instead */
static void __maybe_unused dump_i2c(struct udevice *dev)
{
	int i, c;

	for (i = 0; i < 0x10; i++) {
		printf("00%02x: %02x", i * 0x10, dm_i2c_reg_read(dev, i * 0x10));
		for (c = 1; c < 0xf; c++)
			printf(" %02x", dm_i2c_reg_read(dev, i * 0x10 + c));
		printf(" %02x\n", dm_i2c_reg_read(dev, i * 0x10 + 0xf));
	}
}

static int lm3532_backlight_enable(struct udevice *dev)
{
	struct lm3532_backlight_priv *priv = dev_get_priv(dev);
	int ret, i;

	for (i = 0; i < LM3532_MAX_CONTROL_BANKS; i++) {
		if (priv->bank[i].present) {
			u32 ctrl_en_val = BIT(priv->bank[i].control_bank);

			ret = dm_i2c_reg_clrset(dev, LM3532_REG_ENABLE,
						ctrl_en_val, ctrl_en_val);
			if (ret) {
				log_debug("%s: failed to set ctrl: %d\n",
					  __func__, ret);
				return ret;
			}
		}
	}

	regulator_set_enable_if_allowed(priv->regulator, 1);

	return 0;
}

static int lm3532_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct lm3532_backlight_priv *priv = dev_get_priv(dev);
	struct lm3532_bank_data *bank;
	int ret, i;

	for (i = 0; i < LM3532_MAX_CONTROL_BANKS; i++) {
		if (priv->bank[i].present) {
			bank = &priv->bank[i];
			u32 brightness_reg = LM3532_REG_ZONE_TRGT_A +
					     bank->control_bank * 5 +
					     (bank->ctrl_brt_pointer >> 2);

			ret = dm_i2c_reg_write(dev, brightness_reg, percent);
			if (ret) {
				log_debug("%s: failed to set brightness: %d\n",
					  __func__, ret);
				return ret;
			}
		}
	}

	return 0;
}

static const int ramp_table[LM3532_NUM_RAMP_VALS] = { 8, 1024, 2048, 4096, 8192,
						     16384, 32768, 65536 };
static int lm3532_get_ramp_index(int ramp_time)
{
	int i;

	if (ramp_time <= ramp_table[0])
		return 0;

	if (ramp_time > ramp_table[LM3532_NUM_RAMP_VALS - 1])
		return LM3532_NUM_RAMP_VALS - 1;

	for (i = 1; i < LM3532_NUM_RAMP_VALS; i++) {
		if (ramp_time == ramp_table[i])
			return i;

		/* Find an approximate index by looking up the table */
		if (ramp_time > ramp_table[i - 1] &&
		    ramp_time < ramp_table[i]) {
			if (ramp_time - ramp_table[i - 1] < ramp_table[i] - ramp_time)
				return i - 1;
			else
				return i;
		}
	}

	return 0;
}

static int lm3532_backlight_of_to_plat(struct udevice *dev)
{
	struct lm3532_backlight_priv *priv = dev_get_priv(dev);
	u32 ramp_time, reg;
	ofnode child;
	int ret;

	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not decode enable-gpios (%d)\n", __func__, ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vin-supply", &priv->regulator);
	if (ret) {
		log_debug("%s: vin regulator not defined: %d\n", __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	ramp_time = dev_read_u32_default(dev, "ramp-up-us", 0);
	priv->runtime_ramp_up = lm3532_get_ramp_index(ramp_time);

	ramp_time = dev_read_u32_default(dev, "ramp-down-us", 0);
	priv->runtime_ramp_down = lm3532_get_ramp_index(ramp_time);

	/* Backlight is one of children but has no dedicated driver */
	ofnode_for_each_subnode(child, dev_ofnode(dev)) {
		ret = ofnode_read_u32(child, "reg", &reg);
		if (ret || reg > LM3532_CONTROL_C) {
			log_debug("%s: control bank invalid %d\n", __func__, reg);
			continue;
		}

		struct lm3532_bank_data *bank = &priv->bank[reg];

		bank->control_bank = reg;
		bank->present = 1;
		bank->mode = ofnode_read_u32_default(child, "ti,led-mode",
						     LM3532_BL_MODE_MANUAL);
		bank->mode = LM3532_BL_MODE_ALS ? LM3532_ALS_CTRL : LM3532_I2C_CTRL;

		if (ofnode_read_bool(child, "ti,linear-mapping-mode"))
			bank->mode |= LM3532_LINEAR_MAP;

		bank->num_leds = ofnode_read_size(child, "led-sources");
		bank->num_leds /= sizeof(u32);
		if (bank->num_leds > LM3532_MAX_LED_STRINGS) {
			log_debug("%s: too many LED string defined %d\n",
				  __func__, bank->num_leds);
			continue;
		}

		ret = ofnode_read_u32_array(child, "led-sources",
					    bank->led_strings,
					    bank->num_leds);
		if (ret) {
			log_debug("%s: led-sources property missing %d\n",
				  __func__, ret);
			continue;
		}

		ret = ofnode_read_u32(child, "led-max-microamp",
				      &bank->full_scale_current);
		if (ret)
			log_debug("%s: failed getting led-max-microamp %d\n",
				  __func__, ret);
		else
			bank->full_scale_current = min(bank->full_scale_current,
						       LM3532_FS_CURR_MAX);
	}

	return 0;
}

static int lm3532_backlight_init_registers(struct udevice *dev,
					   struct lm3532_bank_data *bank)
{
	struct lm3532_backlight_priv *priv = dev_get_priv(dev);
	u32 brightness_config_val, runtime_ramp_val;
	u32 output_cfg_val = 0, output_cfg_shift = 0, output_cfg_mask = 0;
	int fs_current_reg, fs_current_val;
	int ret, i;

	if (!bank->present)
		return 0;

	u32 brightness_config_reg = LM3532_REG_ZONE_CFG_A + bank->control_bank * 2;
	/*
	 * This could be hard coded to the default value but the control
	 * brightness register may have changed during boot.
	 */
	ret = dm_i2c_reg_read(dev, brightness_config_reg);
	if (ret < 0)
		return ret;

	bank->ctrl_brt_pointer = ret & ~LM3532_ZONE_MASK;
	brightness_config_val = bank->ctrl_brt_pointer | bank->mode;

	ret = dm_i2c_reg_write(dev, brightness_config_reg, brightness_config_val);
	if (ret)
		return ret;

	if (bank->full_scale_current) {
		fs_current_reg = LM3532_REG_CTRL_A_FS_CURR + bank->control_bank * 2;
		fs_current_val = (bank->full_scale_current - LM3532_FS_CURR_MIN) /
				 LM3532_FS_CURR_STEP;

		ret = dm_i2c_reg_write(dev, fs_current_reg, fs_current_val);
		if (ret)
			return ret;
	}

	for (i = 0; i < bank->num_leds; i++) {
		output_cfg_shift = bank->led_strings[i] * 2;
		output_cfg_val |= (bank->control_bank << output_cfg_shift);
		output_cfg_mask |= LM3532_OUTPUT_CFG_MASK << output_cfg_shift;
	}

	ret = dm_i2c_reg_clrset(dev, LM3532_REG_OUTPUT_CFG, output_cfg_mask,
				output_cfg_val);
	if (ret)
		return ret;

	runtime_ramp_val = priv->runtime_ramp_up |
			 (priv->runtime_ramp_down << LM3532_RAMP_DOWN_SHIFT);

	return dm_i2c_reg_write(dev, LM3532_REG_RT_RAMP, runtime_ramp_val);
}

static int lm3532_backlight_probe(struct udevice *dev)
{
	struct lm3532_backlight_priv *priv = dev_get_priv(dev);
	int ret, i;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C)
		return -EPROTONOSUPPORT;

	dm_gpio_set_value(&priv->enable_gpio, 1);

	for (i = 0; i < LM3532_MAX_CONTROL_BANKS; i++) {
		ret = lm3532_backlight_init_registers(dev, &priv->bank[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct backlight_ops lm3532_backlight_ops = {
	.enable = lm3532_backlight_enable,
	.set_brightness = lm3532_backlight_set_brightness,
};

static const struct udevice_id lm3532_backlight_ids[] = {
	{ .compatible = "ti,lm3532" },
	{ }
};

U_BOOT_DRIVER(lm3532_backlight) = {
	.name		= "lm3532_backlight",
	.id		= UCLASS_PANEL_BACKLIGHT,
	.of_match	= lm3532_backlight_ids,
	.of_to_plat	= lm3532_backlight_of_to_plat,
	.probe		= lm3532_backlight_probe,
	.ops		= &lm3532_backlight_ops,
	.priv_auto	= sizeof(struct lm3532_backlight_priv),
};
