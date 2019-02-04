// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <sysreset.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <power/pmic.h>
#include <power/stpmic1.h>

#define STPMIC1_NUM_OF_REGS 0x100

#if CONFIG_IS_ENABLED(DM_REGULATOR)
static const struct pmic_child_info stpmic1_children_info[] = {
	{ .prefix = "ldo", .driver = "stpmic1_ldo" },
	{ .prefix = "buck", .driver = "stpmic1_buck" },
	{ .prefix = "vref_ddr", .driver = "stpmic1_vref_ddr" },
	{ .prefix = "pwr_sw", .driver = "stpmic1_pwr_sw" },
	{ .prefix = "boost", .driver = "stpmic1_boost" },
	{ },
};
#endif /* DM_REGULATOR */

static int stpmic1_reg_count(struct udevice *dev)
{
	return STPMIC1_NUM_OF_REGS;
}

static int stpmic1_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to write register %#x :%d",
			__func__, reg, ret);

	return ret;
}

static int stpmic1_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "%s: failed to read register %#x : %d",
			__func__, reg, ret);

	return ret;
}

static int stpmic1_bind(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		dev_dbg(dev, "regulators subnode not found!");
		return -ENXIO;
	}
	dev_dbg(dev, "found regulators subnode\n");

	children = pmic_bind_children(dev, regulators_node,
				      stpmic1_children_info);
	if (!children)
		dev_dbg(dev, "no child found\n");
#endif /* DM_REGULATOR */

	if (CONFIG_IS_ENABLED(SYSRESET))
		return device_bind_driver(dev, "stpmic1-sysreset",
					  "stpmic1-sysreset", NULL);

	return 0;
}

static struct dm_pmic_ops stpmic1_ops = {
	.reg_count = stpmic1_reg_count,
	.read = stpmic1_read,
	.write = stpmic1_write,
};

static const struct udevice_id stpmic1_ids[] = {
	{ .compatible = "st,stpmic1" },
	{ }
};

U_BOOT_DRIVER(pmic_stpmic1) = {
	.name = "stpmic1_pmic",
	.id = UCLASS_PMIC,
	.of_match = stpmic1_ids,
	.bind = stpmic1_bind,
	.ops = &stpmic1_ops,
};

#ifdef CONFIG_SYSRESET
static int stpmic1_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct udevice *pmic_dev;
	int ret;

	if (type != SYSRESET_POWER)
		return -EPROTONOSUPPORT;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_stpmic1),
					  &pmic_dev);

	if (ret)
		return -EOPNOTSUPP;

	ret = pmic_reg_read(pmic_dev, STPMIC1_MAIN_CR);
	if (ret < 0)
		return ret;

	ret = pmic_reg_write(pmic_dev, STPMIC1_MAIN_CR,
			     ret | STPMIC1_SWOFF | STPMIC1_RREQ_EN);
	if (ret < 0)
		return ret;

	return -EINPROGRESS;
}

static struct sysreset_ops stpmic1_sysreset_ops = {
	.request = stpmic1_sysreset_request,
};

U_BOOT_DRIVER(stpmic1_sysreset) = {
	.name = "stpmic1-sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &stpmic1_sysreset_ops,
};
#endif
