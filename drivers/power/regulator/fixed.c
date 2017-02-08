/*
 *  Copyright (C) 2015 Samsung Electronics
 *
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

struct fixed_regulator_platdata {
	struct gpio_desc gpio; /* GPIO for regulator enable control */
	unsigned int startup_delay_us;
};

static int fixed_regulator_ofdata_to_platdata(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct fixed_regulator_platdata *dev_pdata;
	struct gpio_desc *gpio;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev), flags = GPIOD_IS_OUT;
	int ret;

	dev_pdata = dev_get_platdata(dev);
	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	/* Set type to fixed */
	uc_pdata->type = REGULATOR_TYPE_FIXED;

	if (fdtdec_get_bool(blob, node, "enable-active-high"))
		flags |= GPIOD_IS_OUT_ACTIVE;

	/* Get fixed regulator optional enable GPIO desc */
	gpio = &dev_pdata->gpio;
	ret = gpio_request_by_name(dev, "gpio", 0, gpio, flags);
	if (ret) {
		debug("Fixed regulator optional enable GPIO - not found! Error: %d\n",
		      ret);
		if (ret != -ENOENT)
			return ret;
	}

	/* Get optional ramp up delay */
	dev_pdata->startup_delay_us = fdtdec_get_uint(gd->fdt_blob,
						      dev_of_offset(dev),
						      "startup-delay-us", 0);

	return 0;
}

static int fixed_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uV != uc_pdata->max_uV) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	return uc_pdata->min_uV;
}

static int fixed_regulator_get_current(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;

	uc_pdata = dev_get_uclass_platdata(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uA != uc_pdata->max_uA) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	return uc_pdata->min_uA;
}

static bool fixed_regulator_get_enable(struct udevice *dev)
{
	struct fixed_regulator_platdata *dev_pdata = dev_get_platdata(dev);

	/* Enable GPIO is optional */
	if (!dev_pdata->gpio.dev)
		return true;

	return dm_gpio_get_value(&dev_pdata->gpio);
}

static int fixed_regulator_set_enable(struct udevice *dev, bool enable)
{
	struct fixed_regulator_platdata *dev_pdata = dev_get_platdata(dev);
	int ret;

	/* Enable GPIO is optional */
	if (!dev_pdata->gpio.dev) {
		if (!enable)
			return -ENOSYS;
		return 0;
	}

	ret = dm_gpio_set_value(&dev_pdata->gpio, enable);
	if (ret) {
		error("Can't set regulator : %s gpio to: %d\n", dev->name,
		      enable);
		return ret;
	}

	if (enable && dev_pdata->startup_delay_us)
		udelay(dev_pdata->startup_delay_us);

	return 0;
}

static const struct dm_regulator_ops fixed_regulator_ops = {
	.get_value	= fixed_regulator_get_value,
	.get_current	= fixed_regulator_get_current,
	.get_enable	= fixed_regulator_get_enable,
	.set_enable	= fixed_regulator_set_enable,
};

static const struct udevice_id fixed_regulator_ids[] = {
	{ .compatible = "regulator-fixed" },
	{ },
};

U_BOOT_DRIVER(fixed_regulator) = {
	.name = "fixed regulator",
	.id = UCLASS_REGULATOR,
	.ops = &fixed_regulator_ops,
	.of_match = fixed_regulator_ids,
	.ofdata_to_platdata = fixed_regulator_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct fixed_regulator_platdata),
};
