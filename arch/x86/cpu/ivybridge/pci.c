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
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	pci_dev_t dev;
	u16 reg16;

	if (!(gd->flags & GD_FLG_RELOC))
		return 0;
	post_code(0x50);
	bd82x6x_init();
	post_code(0x51);

	reg16 = 0xff;
	dev = PCH_DEV;
	reg16 = x86_pci_read_config16(dev, PCI_COMMAND);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	x86_pci_write_config16(dev, PCI_COMMAND, reg16);

	/*
	* Clear non-reserved bits in status register.
	*/
	pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

	pci_write_bar32(hose, dev, 0, 0xf0000000);
	post_code(0x52);

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
