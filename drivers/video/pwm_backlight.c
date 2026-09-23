// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <dm.h>
#include <backlight.h>
#include <limits.h>
#include <log.h>
#include <malloc.h>
#include <pwm.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/math64.h>
#include <power/regulator.h>

/**
 * build_interpolated_levels() - Build a linearly interpolated levels table
 *
 * Some "brightness-levels" tables only list a few anchor points and rely on
 * "num-interpolated-steps" to fill in the values between them, so that a
 * high resolution PWM duty cycle can be used without listing every value.
 *
 * @raw_levels: Anchor point values read from "brightness-levels"
 * @count:	Number of anchor points in @raw_levels
 * @num_steps:	Number of interpolated steps between each anchor point
 * @levels:	Returns newly allocated table of (count - 1) * num_steps + 1
 *		entries
 * @num_levels: Returns the number of entries in @levels
 * Return: 0 on success, -EINVAL if @count or @num_steps are invalid, -ENOMEM
 *	   on allocation failure
 */
static int build_interpolated_levels(const u32 *raw_levels, u32 count,
				     u32 num_steps, u32 **levels,
				     u32 *num_levels)
{
	u64 count_out64;
	u32 count_out, *table, i, x;

	/*
	 * num_steps must fit in the s32 divisor of div_s64(), and count_out64
	 * must fit both u32 (it is stored as such) and the byte size passed
	 * to malloc().
	 */
	if (count < 2 || num_steps == 0 || num_steps > S32_MAX)
		return -EINVAL;

	count_out64 = (u64)(count - 1) * num_steps + 1;
	if (count_out64 > U32_MAX || count_out64 > SIZE_MAX / sizeof(u32))
		return -EINVAL;

	count_out = count_out64;

	table = malloc(count_out * sizeof(u32));
	if (!table)
		return -ENOMEM;

	for (i = 0; i < count - 1; i++) {
		u32 x1 = i * num_steps;
		u32 x2 = x1 + num_steps;
		u32 y1 = raw_levels[i];
		u32 y2 = raw_levels[i + 1];
		s64 dy = (s64)y2 - y1;

		for (x = x1; x < x2; x++)
			table[x] = y1 + div_s64(dy * (x - x1), num_steps);
	}

	table[count_out - 1] = raw_levels[count - 1];
	*levels = table;
	*num_levels = count_out;

	return 0;
}

/**
 * Private information for the PWM backlight
 *
 * If @num_levels is 0 then the levels are simple values with the backlight
 * value going between the minimum (default 0) and the maximum (default 255).
 * Otherwise the levels are an index into @levels (0..n-1).
 *
 * @reg: Regulator to enable to turn the backlight on (NULL if none)
 * @enable, GPIO to set to enable the backlight (can be missing)
 * @pwm: PWM to use to change the backlight brightness
 * @channel: PWM channel to use
 * @period_ns: Period of the backlight in nanoseconds
 * @levels: Levels for the backlight, or NULL if not using indexed levels
 * @num_levels: Number of levels
 * @cur_level: Current level for the backlight (index or value)
 * @default_level: Default level for the backlight (index or value)
 * @min_level: Minimum level of the backlight (full off)
 * @max_level: Maximum level of the backlight (full on)
 * @enabled: true if backlight is enabled
 */
struct pwm_backlight_priv {
	struct udevice *reg;
	struct gpio_desc enable;
	struct udevice *pwm;
	uint channel;
	uint period_ns;
	/*
	 * the polarity of one PWM
	 * 0: normal polarity
	 * 1: inverted polarity
	 */
	bool polarity;
	u32 *levels;
	u32 num_levels;
	uint default_level;
	int cur_level;
	uint min_level;
	uint max_level;
	bool enabled;
};

static int set_pwm(struct pwm_backlight_priv *priv)
{
	u64 width;
	uint duty_cycle;
	int ret;

	if (priv->period_ns) {
		width = priv->period_ns * (priv->cur_level - priv->min_level);
		duty_cycle = div_u64(width,
				     (priv->max_level - priv->min_level));
		ret = pwm_set_config(priv->pwm, priv->channel, priv->period_ns,
				     duty_cycle);
	} else {
		/* PWM driver will internally scale these like the above. */
		ret = pwm_set_config(priv->pwm, priv->channel,
				     priv->max_level - priv->min_level,
				     priv->cur_level - priv->min_level);
	}
	if (ret)
		return log_ret(ret);

	ret = pwm_set_invert(priv->pwm, priv->channel, priv->polarity);
	if (ret == -ENOSYS && !priv->polarity)
		ret = 0;

	return log_ret(ret);
}

static int enable_sequence(struct udevice *dev, int seq)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	switch (seq) {
	case 0:
		if (priv->reg) {
			__maybe_unused struct dm_regulator_uclass_plat
				*plat;

			plat = dev_get_uclass_plat(priv->reg);
			log_debug("Enable '%s', regulator '%s'/'%s'\n",
				  dev->name, priv->reg->name, plat->name);
			ret = regulator_set_enable_if_allowed(priv->reg, true);
			if (ret && ret != -ENOSYS) {
				log_debug("Cannot enable regulator for PWM '%s'\n",
					  dev->name);
				return log_ret(ret);
			}
			mdelay(120);
		}
		break;
	case 1:
		mdelay(10);
		dm_gpio_set_value(&priv->enable, 1);
		break;
	}

	return 0;
}

static int pwm_backlight_enable(struct udevice *dev)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	int ret;

	ret = enable_sequence(dev, 0);
	if (ret)
		return log_ret(ret);
	ret = set_pwm(priv);
	if (ret)
		return log_ret(ret);
	ret = pwm_set_enable(priv->pwm, priv->channel, true);
	if (ret)
		return log_ret(ret);
	ret = enable_sequence(dev, 1);
	if (ret)
		return log_ret(ret);
	priv->enabled = true;

	return 0;
}

static int pwm_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	bool disable = false;
	int level;
	int ret;

	if (!priv->enabled) {
		ret = enable_sequence(dev, 0);
		if (ret)
			return log_ret(ret);
	}
	if (percent == BACKLIGHT_OFF) {
		disable = true;
		percent = 0;
	}
	if (percent == BACKLIGHT_DEFAULT) {
		level = priv->default_level;
	} else {
		if (priv->levels) {
			level = priv->levels[percent * (priv->num_levels - 1)
				/ 100];
		} else {
			level = priv->min_level +
				(priv->max_level - priv->min_level) *
				percent / 100;
		}
	}
	priv->cur_level = level;

	ret = set_pwm(priv);
	if (ret)
		return log_ret(ret);
	if (!priv->enabled) {
		ret = enable_sequence(dev, 1);
		if (ret)
			return log_ret(ret);
		priv->enabled = true;
	}
	if (disable) {
		dm_gpio_set_value(&priv->enable, 0);
		ret = regulator_set_enable_if_allowed(priv->reg, false);
		if (ret && ret != -ENOSYS)
			return log_ret(ret);

		priv->enabled = false;
	}

	return 0;
}

static int pwm_backlight_of_to_plat(struct udevice *dev)
{
	struct pwm_backlight_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	int index, ret, count, len;
	const u32 *cell;
	u32 num_steps;

	log_debug("start\n");
	ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
					   "power-supply", &priv->reg);
	if (ret)
		log_debug("Cannot get power supply: ret=%d\n", ret);
	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (ret) {
		log_debug("Warning: cannot get enable GPIO: ret=%d\n", ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}
	ret = dev_read_phandle_with_args(dev, "pwms", "#pwm-cells", 0, 0,
					 &args);
	if (ret) {
		log_debug("Cannot get PWM phandle: ret=%d\n", ret);
		return log_ret(ret);
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PWM, args.node, &priv->pwm);
	if (ret) {
		log_debug("Cannot get PWM: ret=%d\n", ret);
		return log_ret(ret);
	}
	if (args.args_count < 1)
		return log_msg_ret("Not enough arguments to pwm\n", -EINVAL);
	priv->channel = args.args[0];
	if (args.args_count > 1)
		priv->period_ns = args.args[1];
	if (args.args_count > 2)
		priv->polarity = args.args[2];

	index = dev_read_u32_default(dev, "default-brightness-level", 255);
	cell = dev_read_prop(dev, "brightness-levels", &len);
	count = len / sizeof(u32);

	/*
	 * If present, "num-interpolated-steps" means the levels above are
	 * just anchor points, and the actual table used for
	 * default-brightness-level and PWM duty cycle is the interpolated
	 * table built from those anchor points. Interpolating needs at
	 * least two anchor points.
	 */
	num_steps = dev_read_u32_default(dev, "num-interpolated-steps", 0);

	priv->default_level = index;
	priv->max_level = 255;

	if (cell && count >= (num_steps ? 2 : 1)) {
		u32 *raw_levels;

		raw_levels = malloc(len);
		if (!raw_levels)
			return log_ret(-ENOMEM);

		ret = dev_read_u32_array(dev, "brightness-levels", raw_levels,
					 count);
		if (ret) {
			free(raw_levels);
			return log_msg_ret("levels", ret);
		}

		if (num_steps) {
			ret = build_interpolated_levels(raw_levels, count, num_steps,
							&priv->levels,
							&priv->num_levels);
			free(raw_levels);
			if (ret)
				return log_ret(ret);
		} else {
			priv->levels = raw_levels;
			priv->num_levels = count;
		}

		if (index < priv->num_levels) {
			priv->default_level = priv->levels[index];
			priv->max_level = priv->levels[priv->num_levels - 1];
		} else {
			/*
			 * default-brightness-level is out of range for the
			 * table: fall back to raw 0-255 PWM scaling instead
			 * of using the table at all.
			 */
			log_warning("default-brightness-level %d out of range for %u-entry brightness-levels table, ignoring table\n",
				    index, priv->num_levels);
			free(priv->levels);
			priv->levels = NULL;
			priv->num_levels = 0;
		}
	}
	priv->cur_level = priv->default_level;
	log_debug("done\n");

	return 0;
}

static int pwm_backlight_probe(struct udevice *dev)
{
	return 0;
}

static const struct backlight_ops pwm_backlight_ops = {
	.enable		= pwm_backlight_enable,
	.set_brightness	= pwm_backlight_set_brightness,
};

static const struct udevice_id pwm_backlight_ids[] = {
	{ .compatible = "pwm-backlight" },
	{ }
};

U_BOOT_DRIVER(pwm_backlight) = {
	.name	= "pwm_backlight",
	.id	= UCLASS_PANEL_BACKLIGHT,
	.of_match = pwm_backlight_ids,
	.ops	= &pwm_backlight_ops,
	.of_to_plat	= pwm_backlight_of_to_plat,
	.probe		= pwm_backlight_probe,
	.priv_auto	= sizeof(struct pwm_backlight_priv),
};
