// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/s2mps11.h>

static const struct pmic_child_info s2mps11_pmic_children_info[] = {
	{ .prefix = S2MPS11_OF_LDO_PREFIX, .driver = S2MPS11_LDO_DRIVER },
	{ .prefix = S2MPS11_OF_BUCK_PREFIX, .driver = S2MPS11_BUCK_DRIVER },
	{ },
};

static const struct pmic_child_info s2mpu05_pmic_children_info[] = {
	{ .prefix = S2MPU05_OF_LDO_PREFIX, .driver = S2MPS11_LDO_DRIVER },
	{ .prefix = S2MPU05_OF_BUCK_PREFIX, .driver = S2MPS11_BUCK_DRIVER },
	{ },
};

static int s2mps11_reg_count(struct udevice *dev)
{
	switch (dev_get_driver_data(dev)) {
	case VARIANT_S2MPS11:
		return S2MPS11_REG_COUNT;
	case VARIANT_S2MPU05:
		return S2MPU05_REG_COUNT;
	default:
		return -EINVAL;
	}
}

static int s2mps11_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		pr_err("write error to device: %p register: %#x!\n", dev, reg);

	return ret;
}

static int s2mps11_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		pr_err("read error from device: %p register: %#x!\n", dev, reg);

	return ret;
}

static int s2mps11_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;
	const struct pmic_child_info *pmic_children_info;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	switch (dev_get_driver_data(dev)) {
	case VARIANT_S2MPS11:
		pmic_children_info = s2mps11_pmic_children_info;
		break;
	case VARIANT_S2MPU05:
		pmic_children_info = s2mpu05_pmic_children_info;
		break;
	default:
		debug("%s: unknown device type\n", __func__);
		return -EINVAL;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	return 0;
}

static struct dm_pmic_ops s2mps11_ops = {
	.reg_count = s2mps11_reg_count,
	.read = s2mps11_read,
	.write = s2mps11_write,
};

static const struct udevice_id s2mps11_ids[] = {
	{ .compatible = "samsung,s2mps11-pmic", .data = VARIANT_S2MPS11 },
	{ .compatible = "samsung,s2mpu05-pmic", .data = VARIANT_S2MPU05 },
	{ }
};

U_BOOT_DRIVER(pmic_s2mps11) = {
	.name = "s2mps11_pmic",
	.id = UCLASS_PMIC,
	.of_match = s2mps11_ids,
	.ops = &s2mps11_ops,
	.bind = s2mps11_bind,
};
