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
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/arch/bd82x6x.h>
#include <asm/arch/pch.h>

static int pci_ivybridge_probe(struct udevice *bus)
{
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;
	post_code(0x50);
	bd82x6x_init_extra();
	post_code(0x51);

	return 0;
}

static const struct dm_pci_ops pci_ivybridge_ops = {
	.read_config	= pci_x86_read_config,
	.write_config	= pci_x86_write_config,
};

static const struct udevice_id pci_ivybridge_ids[] = {
	{ .compatible = "intel,pci-ivybridge" },
	{ }
};

U_BOOT_DRIVER(pci_ivybridge_drv) = {
	.name		= "pci_ivybridge",
	.id		= UCLASS_PCI,
	.of_match	= pci_ivybridge_ids,
	.ops		= &pci_ivybridge_ops,
	.probe		= pci_ivybridge_probe,
};
