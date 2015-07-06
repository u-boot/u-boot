/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mmc.h>
#include <asm/test.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct udevice_id sandbox_mmc_ids[] = {
	{ .compatible = "sandbox,mmc" },
	{ }
};

U_BOOT_DRIVER(warm_mmc_sandbox) = {
	.name		= "mmc_sandbox",
	.id		= UCLASS_MMC,
	.of_match	= sandbox_mmc_ids,
};
