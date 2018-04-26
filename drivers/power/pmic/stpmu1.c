// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/stpmu1.h>

#define STMPU1_NUM_OF_REGS 0x100

#ifndef CONFIG_SPL_BUILD
static const struct pmic_child_info stpmu1_children_info[] = {
	{ .prefix = "ldo", .driver = "stpmu1_ldo" },
	{ .prefix = "buck", .driver = "stpmu1_buck" },
	{ .prefix = "vref_ddr", .driver = "stpmu1_vref_ddr" },
	{ .prefix = "pwr_sw", .driver = "stpmu1_pwr_sw" },
	{ .prefix = "boost", .driver = "stpmu1_boost" },
	{ },
};
#endif /* CONFIG_SPL_BUILD */

static int stpmu1_reg_count(struct udevice *dev)
{
	return STMPU1_NUM_OF_REGS;
}

static int stpmu1_write(struct udevice *dev, uint reg, const uint8_t *buff,
			int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to write register %#x :%d",
			__func__, reg, ret);

	return ret;
}

static int stpmu1_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to read register %#x : %d",
			__func__, reg, ret);

	return ret;
}

static int stpmu1_bind(struct udevice *dev)
{
#ifndef CONFIG_SPL_BUILD
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		dev_dbg(dev, "regulators subnode not found!");
		return -ENXIO;
	}
	dev_dbg(dev, "found regulators subnode\n");

	children = pmic_bind_children(dev, regulators_node,
				      stpmu1_children_info);
	if (!children)
		dev_dbg(dev, "no child found\n");
#endif /* CONFIG_SPL_BUILD */

	return 0;
}

static struct dm_pmic_ops stpmu1_ops = {
	.reg_count = stpmu1_reg_count,
	.read = stpmu1_read,
	.write = stpmu1_write,
};

static const struct udevice_id stpmu1_ids[] = {
	{ .compatible = "st,stpmu1" },
	{ }
};

U_BOOT_DRIVER(pmic_stpmu1) = {
	.name = "stpmu1_pmic",
	.id = UCLASS_PMIC,
	.of_match = stpmu1_ids,
	.bind = stpmu1_bind,
	.ops = &stpmu1_ops,
};
