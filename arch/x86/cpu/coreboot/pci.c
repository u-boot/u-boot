/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008,2009
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <pci.h>

static const struct udevice_id generic_pch_ids[] = {
	{ .compatible = "intel,pch7" },
	{ .compatible = "intel,pch9" },
	{ }
};

U_BOOT_DRIVER(generic_pch_drv) = {
	.name		= "pch",
	.id		= UCLASS_PCH,
	.of_match	= generic_pch_ids,
};
