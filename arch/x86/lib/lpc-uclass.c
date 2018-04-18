/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>

UCLASS_DRIVER(lpc) = {
	.id		= UCLASS_LPC,
	.name		= "lpc",
	.post_bind	= dm_scan_fdt_dev,
};
