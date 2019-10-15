// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/pca9450.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
	/* buck */
	{ .prefix = "b", .driver = PCA9450_REGULATOR_DRIVER},
	/* ldo */
	{ .prefix = "l", .driver = PCA9450_REGULATOR_DRIVER},
	{ },
};

static int pca9450_reg_count(struct udevice *dev)
{
	return PCA9450_REG_NUM;
}

static int pca9450_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pca9450_read(struct udevice *dev, uint reg, uint8_t *buff,
			int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pca9450_bind(struct udevice *dev)
{
	int children;
	ofnode regulators_node;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node,
				      pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops pca9450_ops = {
	.reg_count = pca9450_reg_count,
	.read = pca9450_read,
	.write = pca9450_write,
};

static const struct udevice_id pca9450_ids[] = {
	{ .compatible = "nxp,pca9450a", .data = 0x35, },
	{ .compatible = "nxp,pca9450b", .data = 0x25, },
	{ }
};

U_BOOT_DRIVER(pmic_pca9450) = {
	.name = "pca9450 pmic",
	.id = UCLASS_PMIC,
	.of_match = pca9450_ids,
	.bind = pca9450_bind,
	.ops = &pca9450_ops,
};
