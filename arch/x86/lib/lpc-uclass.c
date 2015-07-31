/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

static int lpc_uclass_post_bind(struct udevice *bus)
{
	/*
	 * Scan the device tree for devices
	 *
	 * Before relocation, only bind devices marked for pre-relocation
	 * use.
	 */
	return dm_scan_fdt_node(bus, gd->fdt_blob, bus->of_offset,
				gd->flags & GD_FLG_RELOC ? false : true);
}

UCLASS_DRIVER(lpc) = {
	.id		= UCLASS_LPC,
	.name		= "lpc",
	.post_bind	= lpc_uclass_post_bind,
};
