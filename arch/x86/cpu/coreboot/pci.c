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
#include <pci.h>
#include <asm/pci.h>

DECLARE_GLOBAL_DATA_PTR;

static void config_pci_bridge(struct pci_controller *hose, pci_dev_t dev,
			      struct pci_config_table *table)
{
	u8 secondary;
	hose->read_byte(hose, dev, PCI_SECONDARY_BUS, &secondary);
	hose->last_busno = max(hose->last_busno, (int)secondary);
	pci_hose_scan_bus(hose, secondary);
}

static struct pci_config_table pci_coreboot_config_table[] = {
	/* vendor, device, class, bus, dev, func */
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_BRIDGE_PCI,
		PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, &config_pci_bridge },
	{}
};

void board_pci_setup_hose(struct pci_controller *hose)
{
	hose->config_table = pci_coreboot_config_table;
	hose->first_busno = 0;
	hose->last_busno = 0;

	/* PCI memory space */
	pci_set_region(hose->regions + 0,
		       CONFIG_PCI_MEM_BUS,
		       CONFIG_PCI_MEM_PHYS,
		       CONFIG_PCI_MEM_SIZE,
		       PCI_REGION_MEM);

	/* PCI IO space */
	pci_set_region(hose->regions + 1,
		       CONFIG_PCI_IO_BUS,
		       CONFIG_PCI_IO_PHYS,
		       CONFIG_PCI_IO_SIZE,
		       PCI_REGION_IO);

	pci_set_region(hose->regions + 2,
		       CONFIG_PCI_PREF_BUS,
		       CONFIG_PCI_PREF_PHYS,
		       CONFIG_PCI_PREF_SIZE,
		       PCI_REGION_PREFETCH);

	pci_set_region(hose->regions + 3,
		       0,
		       0,
		       gd->ram_size,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	hose->region_count = 4;
}
