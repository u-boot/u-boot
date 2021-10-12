// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>

static int axp_pmic_reg_count(struct udevice *dev)
{
	/* TODO: Get the specific value from driver data. */
	return 0x100;
}

static struct dm_pmic_ops axp_pmic_ops = {
	.reg_count	= axp_pmic_reg_count,
	.read		= dm_i2c_read,
	.write		= dm_i2c_write,
};

static const struct udevice_id axp_pmic_ids[] = {
	{ .compatible = "x-powers,axp152" },
	{ .compatible = "x-powers,axp202" },
	{ .compatible = "x-powers,axp209" },
	{ .compatible = "x-powers,axp221" },
	{ .compatible = "x-powers,axp223" },
	{ .compatible = "x-powers,axp803" },
	{ .compatible = "x-powers,axp806" },
	{ .compatible = "x-powers,axp809" },
	{ .compatible = "x-powers,axp813" },
	{ }
};

U_BOOT_DRIVER(axp_pmic) = {
	.name		= "axp_pmic",
	.id		= UCLASS_PMIC,
	.of_match	= axp_pmic_ids,
	.bind		= dm_scan_fdt_dev,
	.ops		= &axp_pmic_ops,
};
