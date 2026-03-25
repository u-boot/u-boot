// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025-2026 RISCstar Ltd.
 */

#include <dm.h>
#include <power/pmic.h>
#include <power/spacemit_p1.h>

static int pmic_p1_reg_count(struct udevice *dev)
{
	return P1_MAX_REGS;
}

static int pmic_p1_write(struct udevice *dev, uint reg, const u8 *buffer,
			 int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buffer, len);
	if (ret)
		pr_err("%s write error on register %02x\n", dev->name, reg);

	return ret;
}

static int pmic_p1_read(struct udevice *dev, uint reg, u8 *buffer,
			int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buffer, len);
	if (ret)
		pr_err("%s read error on register %02x\n", dev->name, reg);

	return ret;
}

static const struct pmic_child_info p1_children_info[] = {
	{ .prefix = "buck",		.driver = P1_BUCK_DRIVER },
	{ .prefix = "aldo",		.driver = P1_ALDO_DRIVER },
	{ .prefix = "dldo",		.driver = P1_DLDO_DRIVER },
	{ },
};

static int pmic_p1_bind(struct udevice *dev)
{
	const struct pmic_child_info *p1_children_info =
			(struct pmic_child_info *)dev_get_driver_data(dev);
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s regulators subnode not found\n", dev->name);
		return -EINVAL;
	}

	children = pmic_bind_children(dev, regulators_node,
				      p1_children_info);
	if (!children)
		debug("%s has no children (regulators)\n", dev->name);

	return 0;
}

static int pmic_p1_probe(struct udevice *dev)
{
	return 0;
}

static struct dm_pmic_ops pmic_p1_ops = {
	.reg_count	= pmic_p1_reg_count,
	.read		= pmic_p1_read,
	.write		= pmic_p1_write,
};

static const struct udevice_id pmic_p1_match[] = {
	{
		.compatible = "spacemit,p1",
		.data = (ulong)&p1_children_info,
	}, {
		/* sentinel */
	}
};

U_BOOT_DRIVER(pmic_p1) = {
	.name		= "pmic_p1",
	.id		= UCLASS_PMIC,
	.of_match	= pmic_p1_match,
	.bind		= pmic_p1_bind,
	.probe		= pmic_p1_probe,
	.ops		= &pmic_p1_ops,
};
