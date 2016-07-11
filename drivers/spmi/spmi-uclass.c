/*
 * SPMI bus uclass driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <dm/root.h>
#include <spmi/spmi.h>
#include <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

int spmi_reg_read(struct udevice *dev, int usid, int pid, int reg)
{
	const struct dm_spmi_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->read)
		return -ENOSYS;

	return ops->read(dev, usid, pid, reg);
}

int spmi_reg_write(struct udevice *dev, int usid, int pid, int reg,
		   uint8_t value)
{
	const struct dm_spmi_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->write)
		return -ENOSYS;

	return ops->write(dev, usid, pid, reg, value);
}

static int spmi_post_bind(struct udevice *dev)
{
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

UCLASS_DRIVER(spmi) = {
	.id		= UCLASS_SPMI,
	.name		= "spmi",
	.post_bind	= spmi_post_bind,
};
