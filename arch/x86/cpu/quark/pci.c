/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <pci.h>
#include <asm/pci.h>
#include <asm/arch/device.h>

DECLARE_GLOBAL_DATA_PTR;

void board_pci_setup_hose(struct pci_controller *hose)
{
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

int board_pci_post_scan(struct pci_controller *hose)
{
	return 0;
}

int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	/*
	 * TODO:
	 *
	 * For some unknown reason, the PCI enumeration process hangs
	 * when it scans to the PCIe root port 0 (D23:F0) & 1 (D23:F1).
	 *
	 * For now we just skip these two devices, and this needs to
	 * be revisited later.
	 */
	if (dev == QUARK_HOST_BRIDGE ||
	    dev == QUARK_PCIE0 || dev == QUARK_PCIE1) {
		return 1;
	}

	return 0;
}
