/*
 * PCI auto-configuration library
 *
 * Author: Matt Porter <mporter@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 *
 * Modifications for driver model:
 * Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>

void pciauto_region_init(struct pci_region *res)
{
	/*
	 * Avoid allocating PCI resources from address 0 -- this is illegal
	 * according to PCI 2.1 and moreover, this is known to cause Linux IDE
	 * drivers to fail. Use a reasonable starting value of 0x1000 instead.
	 */
	res->bus_lower = res->bus_start ? res->bus_start : 0x1000;
}

void pciauto_region_align(struct pci_region *res, pci_size_t size)
{
	res->bus_lower = ((res->bus_lower - 1) | (size - 1)) + 1;
}

int pciauto_region_allocate(struct pci_region *res, pci_size_t size,
	pci_addr_t *bar)
{
	pci_addr_t addr;

	if (!res) {
		debug("No resource");
		goto error;
	}

	addr = ((res->bus_lower - 1) | (size - 1)) + 1;

	if (addr - res->bus_start + size > res->size) {
		debug("No room in resource");
		goto error;
	}

	res->bus_lower = addr + size;

	debug("address=0x%llx bus_lower=0x%llx", (unsigned long long)addr,
	      (unsigned long long)res->bus_lower);

	*bar = addr;
	return 0;

 error:
	*bar = (pci_addr_t)-1;
	return -1;
}

void pciauto_config_init(struct pci_controller *hose)
{
	int i;

	hose->pci_io = NULL;
	hose->pci_mem = NULL;
	hose->pci_prefetch = NULL;

	for (i = 0; i < hose->region_count; i++) {
		switch (hose->regions[i].flags) {
		case PCI_REGION_IO:
			if (!hose->pci_io ||
			    hose->pci_io->size < hose->regions[i].size)
				hose->pci_io = hose->regions + i;
			break;
		case PCI_REGION_MEM:
			if (!hose->pci_mem ||
			    hose->pci_mem->size < hose->regions[i].size)
				hose->pci_mem = hose->regions + i;
			break;
		case (PCI_REGION_MEM | PCI_REGION_PREFETCH):
			if (!hose->pci_prefetch ||
			    hose->pci_prefetch->size < hose->regions[i].size)
				hose->pci_prefetch = hose->regions + i;
			break;
		}
	}


	if (hose->pci_mem) {
		pciauto_region_init(hose->pci_mem);

		debug("PCI Autoconfig: Bus Memory region: [0x%llx-0x%llx],\n"
		       "\t\tPhysical Memory [%llx-%llxx]\n",
		    (u64)hose->pci_mem->bus_start,
		    (u64)(hose->pci_mem->bus_start + hose->pci_mem->size - 1),
		    (u64)hose->pci_mem->phys_start,
		    (u64)(hose->pci_mem->phys_start + hose->pci_mem->size - 1));
	}

	if (hose->pci_prefetch) {
		pciauto_region_init(hose->pci_prefetch);

		debug("PCI Autoconfig: Bus Prefetchable Mem: [0x%llx-0x%llx],\n"
		       "\t\tPhysical Memory [%llx-%llx]\n",
		    (u64)hose->pci_prefetch->bus_start,
		    (u64)(hose->pci_prefetch->bus_start +
			    hose->pci_prefetch->size - 1),
		    (u64)hose->pci_prefetch->phys_start,
		    (u64)(hose->pci_prefetch->phys_start +
			    hose->pci_prefetch->size - 1));
	}

	if (hose->pci_io) {
		pciauto_region_init(hose->pci_io);

		debug("PCI Autoconfig: Bus I/O region: [0x%llx-0x%llx],\n"
		       "\t\tPhysical Memory: [%llx-%llx]\n",
		    (u64)hose->pci_io->bus_start,
		    (u64)(hose->pci_io->bus_start + hose->pci_io->size - 1),
		    (u64)hose->pci_io->phys_start,
		    (u64)(hose->pci_io->phys_start + hose->pci_io->size - 1));
	}
}
