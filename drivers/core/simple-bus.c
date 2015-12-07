/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

struct simple_bus_plat {
	u32 base;
	u32 size;
	u32 target;
};

fdt_addr_t simple_bus_translate(struct udevice *dev, fdt_addr_t addr)
{
	struct simple_bus_plat *plat = dev_get_uclass_platdata(dev);

	if (addr >= plat->base && addr < plat->base + plat->size)
		addr = (addr - plat->base) + plat->target;

	return addr;
}

static int simple_bus_post_bind(struct udevice *dev)
{
	u32 cell[3];
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, dev->of_offset, "ranges",
				   cell, ARRAY_SIZE(cell));
	if (!ret) {
		struct simple_bus_plat *plat = dev_get_uclass_platdata(dev);

		plat->base = cell[0];
		plat->target = cell[1];
		plat->size = cell[2];
	}

	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

UCLASS_DRIVER(simple_bus) = {
	.id		= UCLASS_SIMPLE_BUS,
	.name		= "simple_bus",
	.post_bind	= simple_bus_post_bind,
	.per_device_platdata_auto_alloc_size = sizeof(struct simple_bus_plat),
};

static const struct udevice_id generic_simple_bus_ids[] = {
	{ .compatible = "simple-bus" },
	{ }
};

U_BOOT_DRIVER(simple_bus_drv) = {
	.name	= "generic_simple_bus",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = generic_simple_bus_ids,
};
