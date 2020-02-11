// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Disruptive Technologies Research AS
 * Sven Schwermer <sven.svenschwermer@disruptive-technologies.com>
 */

#include "regulator_common.h"
#include <common.h>
#include <power/regulator.h>

int regulator_common_ofdata_to_platdata(struct udevice *dev,
	struct regulator_common_platdata *dev_pdata, const char *enable_gpio_name)
{
	struct gpio_desc *gpio;
	int flags = GPIOD_IS_OUT;
	int ret;

	if (!dev_read_bool(dev, "enable-active-high"))
		flags |= GPIOD_ACTIVE_LOW;

	/* Get optional enable GPIO desc */
	gpio = &dev_pdata->gpio;
	ret = gpio_request_by_name(dev, enable_gpio_name, 0, gpio, flags);
	if (ret) {
		debug("Regulator '%s' optional enable GPIO - not found! Error: %d\n",
		      dev->name, ret);
		if (ret != -ENOENT)
			return ret;
	}

	/* Get optional ramp up delay */
	dev_pdata->startup_delay_us = dev_read_u32_default(dev,
							"startup-delay-us", 0);
	dev_pdata->off_on_delay_us =
		dev_read_u32_default(dev, "off-on-delay-us", 0);
	if (!dev_pdata->off_on_delay_us) {
		dev_pdata->off_on_delay_us =
			dev_read_u32_default(dev, "u-boot,off-on-delay-us", 0);
	}

	return 0;
}

int regulator_common_get_enable(const struct udevice *dev,
	struct regulator_common_platdata *dev_pdata)
{
	/* Enable GPIO is optional */
	if (!dev_pdata->gpio.dev)
		return true;

	return dm_gpio_get_value(&dev_pdata->gpio);
}

int regulator_common_set_enable(const struct udevice *dev,
	struct regulator_common_platdata *dev_pdata, bool enable)
{
	int ret;

	debug("%s: dev='%s', enable=%d, delay=%d, has_gpio=%d\n", __func__,
	      dev->name, enable, dev_pdata->startup_delay_us,
	      dm_gpio_is_valid(&dev_pdata->gpio));
	/* Enable GPIO is optional */
	if (!dm_gpio_is_valid(&dev_pdata->gpio)) {
		if (!enable)
			return -ENOSYS;
		return 0;
	}

	ret = dm_gpio_set_value(&dev_pdata->gpio, enable);
	if (ret) {
		pr_err("Can't set regulator : %s gpio to: %d\n", dev->name,
		      enable);
		return ret;
	}

	if (enable && dev_pdata->startup_delay_us)
		udelay(dev_pdata->startup_delay_us);
	debug("%s: done\n", __func__);

	if (!enable && dev_pdata->off_on_delay_us)
		udelay(dev_pdata->off_on_delay_us);

	return 0;
}
