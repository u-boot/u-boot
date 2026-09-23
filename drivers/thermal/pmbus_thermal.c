// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Generic UCLASS_THERMAL companion for PMBus voltage regulators.
 *
 * Works with any chip bound by a pmbus_helper based regulator driver
 * (drivers/power/regulator/<chip>.c calling
 * pmbus_regulator_probe_common()). It auto-spawns one
 * of these thermal devices per regulator.
 *
 * The reading is the chip's READ_TEMPERATURE_1 (PMBus 0x8D), decoded
 * through the parent's pmbus_driver_info.
 */

#include <dm.h>
#include <pmbus.h>
#include <thermal.h>

static int pmbus_thermal_get_temp(struct udevice *dev, int *temp)
{
	return pmbus_regulator_read_temp(dev_get_parent(dev), temp);
}

static const struct dm_thermal_ops pmbus_thermal_ops = {
	.get_temp = pmbus_thermal_get_temp,
};

U_BOOT_DRIVER(pmbus_thermal) = {
	.name	= "pmbus_thermal",
	.id	= UCLASS_THERMAL,
	.ops	= &pmbus_thermal_ops,
};
