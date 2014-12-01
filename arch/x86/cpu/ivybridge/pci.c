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
#include <asm/arch/bd82x6x.h>
#include <asm/arch/pch.h>

static void config_pci_bridge(struct pci_controller *hose, pci_dev_t dev,
			      struct pci_config_table *table)
{
	u8 secondary;

	hose->read_byte(hose, dev, PCI_SECONDARY_BUS, &secondary);
	if (secondary != 0)
		pci_hose_scan_bus(hose, secondary);
}

static struct pci_config_table pci_ivybridge_config_table[] = {
	/* vendor, device, class, bus, dev, func */
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_BRIDGE_PCI,
		PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, &config_pci_bridge },
	{}
};

void board_pci_setup_hose(struct pci_controller *hose)
{
	hose->config_table = pci_ivybridge_config_table;
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

	hose->region_count = 3;
}

int board_pci_pre_scan(struct pci_controller *hose)
{
	pci_dev_t dev;
	u16 reg16;

	bd82x6x_init();

	reg16 = 0xff;
	dev = PCH_DEV;
	reg16 = pci_read_config16(dev, PCI_COMMAND);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config16(dev, PCI_COMMAND, reg16);

	/*
	* Clear non-reserved bits in status register.
	*/
	pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

	pci_write_bar32(hose, dev, 0, 0xf0000000);

	return 0;
}

int board_pci_post_scan(struct pci_controller *hose)
{
	int ret;

	ret = bd82x6x_init_pci_devices();
	if (ret) {
		printf("bd82x6x_init_pci_devices() failed: %d\n", ret);
		return ret;
	}

	return 0;
}
