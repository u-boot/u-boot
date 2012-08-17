/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <pci.h>
#include <usb.h>

#include "ehci.h"
#include "ehci-core.h"

#ifdef CONFIG_PCI_EHCI_DEVICE
static struct pci_device_id ehci_pci_ids[] = {
	/* Please add supported PCI EHCI controller ids here */
	{0x1033, 0x00E0},	/* NEC */
	{0x10B9, 0x5239},	/* ULI1575 PCI EHCI module ids */
	{0x12D8, 0x400F},	/* Pericom */
	{0, 0}
};
#endif

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(void)
{
	pci_dev_t pdev;

	pdev = pci_find_devices(ehci_pci_ids, CONFIG_PCI_EHCI_DEVICE);
	if (pdev == -1) {
		printf("EHCI host controller not found\n");
		return -1;
	}

	hccr = (struct ehci_hccr *)pci_map_bar(pdev,
			PCI_BASE_ADDRESS_0, PCI_REGION_MEM);
	hcor = (struct ehci_hcor *)((uint32_t) hccr +
			HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	debug("EHCI-PCI init hccr 0x%x and hcor 0x%x hc_length %d\n",
			(uint32_t)hccr, (uint32_t)hcor,
			(uint32_t)HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}
