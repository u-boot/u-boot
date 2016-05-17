/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>

UCLASS_DRIVER(ahci) = {
	.id		= UCLASS_AHCI,
	.name		= "ahci",
};
