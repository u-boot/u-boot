/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <power/rk808_pmic.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "DCDC_REG", .driver = "rk808_buck"},
	{ .prefix = "LDO_REG", .driver = "rk808_ldo"},
	{ .prefix = "SWITCH_REG", .driver = "rk808_switch"},
	{ },
};

static int rk808_reg_count(struct udevice *dev)
{
	return RK808_NUM_OF_REGS;
}

static int rk808_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret) {
		debug("write error to device: %p register: %#x!", dev, reg);
		return ret;
	}

	return 0;
}

static int rk808_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		debug("read error from device: %p register: %#x!", dev, reg);
		return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
static int rk808_bind(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	int regulators_node;
	int children;

	regulators_node = fdt_subnode_offset(blob, dev_of_offset(dev),
					     "regulators");
	if (regulators_node <= 0) {
		debug("%s: %s regulators subnode not found!", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}
#endif

static struct dm_pmic_ops rk808_ops = {
	.reg_count = rk808_reg_count,
	.read = rk808_read,
	.write = rk808_write,
};

static const struct udevice_id rk808_ids[] = {
	{ .compatible = "rockchip,rk808" },
	{ }
};

U_BOOT_DRIVER(pmic_rk808) = {
	.name = "rk808 pmic",
	.id = UCLASS_PMIC,
	.of_match = rk808_ids,
#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
	.bind = rk808_bind,
#endif
	.ops = &rk808_ops,
};
