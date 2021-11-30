// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *
 *  Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <power/pmic.h>
#include <power/regulator.h>

#include "regulator_common.h"

static int fixed_regulator_of_to_plat(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct regulator_common_plat *dev_pdata;

	dev_pdata = dev_get_plat(dev);
	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	uc_pdata->type = REGULATOR_TYPE_FIXED;

	return regulator_common_of_to_plat(dev, dev_pdata, "gpio");
}

static int fixed_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
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
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	if (uc_pdata->min_uA != uc_pdata->max_uA) {
		debug("Invalid constraints for: %s\n", uc_pdata->name);
		return -EINVAL;
	}

	return uc_pdata->min_uA;
}

static int fixed_regulator_get_enable(struct udevice *dev)
{
	return regulator_common_get_enable(dev, dev_get_plat(dev));
}

static int fixed_regulator_set_enable(struct udevice *dev, bool enable)
{
	return regulator_common_set_enable(dev, dev_get_plat(dev), enable);
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

U_BOOT_DRIVER(regulator_fixed) = {
	.name = "regulator_fixed",
	.id = UCLASS_REGULATOR,
	.ops = &fixed_regulator_ops,
	.of_match = fixed_regulator_ids,
	.of_to_plat = fixed_regulator_of_to_plat,
	.plat_auto	= sizeof(struct regulator_common_plat),
};
