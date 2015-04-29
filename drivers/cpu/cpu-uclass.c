/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/root.h>

int cpu_get_desc(struct udevice *dev, char *buf, int size)
{
	struct cpu_ops *ops = cpu_get_ops(dev);

	if (!ops->get_desc)
		return -ENOSYS;

	return ops->get_desc(dev, buf, size);
}

int cpu_get_info(struct udevice *dev, struct cpu_info *info)
{
	struct cpu_ops *ops = cpu_get_ops(dev);

	if (!ops->get_desc)
		return -ENOSYS;

	return ops->get_info(dev, info);
}

U_BOOT_DRIVER(cpu_bus) = {
	.name	= "cpu_bus",
	.id	= UCLASS_SIMPLE_BUS,
	.per_child_platdata_auto_alloc_size = sizeof(struct cpu_platdata),
};

static int uclass_cpu_init(struct uclass *uc)
{
	struct udevice *dev;
	int node;
	int ret;

	node = fdt_path_offset(gd->fdt_blob, "/cpus");
	if (node < 0)
		return 0;

	ret = device_bind_driver_to_node(dm_root(), "cpu_bus", "cpus", node,
					 &dev);

	return ret;
}

UCLASS_DRIVER(cpu) = {
	.id		= UCLASS_CPU,
	.name		= "cpu",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.init		= uclass_cpu_init,
};
