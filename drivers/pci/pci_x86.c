/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <pci.h>

static const struct dm_pci_ops x86_pci_ops = {
};

static const struct udevice_id x86_pci_ids[] = {
	{ .compatible = "x86,pci" },
	{ }
};

U_BOOT_DRIVER(pci_x86) = {
	.name	= "pci_x86",
	.id	= UCLASS_PCI,
	.of_match = x86_pci_ids,
	.ops	= &x86_pci_ops,
};
