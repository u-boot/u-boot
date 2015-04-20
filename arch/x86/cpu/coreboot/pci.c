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
#include <errno.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/pci.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct dm_pci_ops pci_x86_ops = {
	.read_config	= pci_x86_read_config,
	.write_config	= pci_x86_write_config,
};

static const struct udevice_id pci_x86_ids[] = {
	{ .compatible = "pci-x86" },
	{ }
};

U_BOOT_DRIVER(pci_x86_drv) = {
	.name		= "pci_x86",
	.id		= UCLASS_PCI,
	.of_match	= pci_x86_ids,
	.ops		= &pci_x86_ops,
};

static const struct udevice_id generic_pch_ids[] = {
	{ .compatible = "intel,pch" },
	{ }
};

U_BOOT_DRIVER(generic_pch_drv) = {
	.name		= "pch",
	.id		= UCLASS_PCH,
	.of_match	= generic_pch_ids,
};
