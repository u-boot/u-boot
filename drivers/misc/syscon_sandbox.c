/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/test.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct udevice_id sandbox_syscon_ids[] = {
	{ .compatible = "sandbox,syscon0", .data = SYSCON0 },
	{ .compatible = "sandbox,syscon1", .data = SYSCON1 },
	{ }
};

U_BOOT_DRIVER(sandbox_syscon) = {
	.name	= "sandbox_syscon",
	.id	= UCLASS_SYSCON,
	.of_match = sandbox_syscon_ids,
};
