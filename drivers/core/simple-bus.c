/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

static int simple_bus_post_bind(struct udevice *dev)
{
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

UCLASS_DRIVER(simple_bus) = {
	.id		= UCLASS_SIMPLE_BUS,
	.name		= "simple_bus",
	.post_bind	= simple_bus_post_bind,
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
