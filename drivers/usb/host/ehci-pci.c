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
#include <errno.h>
#include <pci.h>
#include <usb.h>

#include "ehci.h"

#ifdef CONFIG_PCI_EHCI_DEVICE
static struct pci_device_id ehci_pci_ids[] = {
	/* Please add supported PCI EHCI controller ids here */
	{0x1033, 0x00E0},	/* NEC */
	{0x10B9, 0x5239},	/* ULI1575 PCI EHCI module ids */
	{0x12D8, 0x400F},	/* Pericom */
	{0, 0}
};
#else
static pci_dev_t ehci_find_class(int index)
{
	int bus;
	int devnum;
	pci_dev_t bdf;
	uint32_t class;

	for (bus = 0; bus <= pci_last_busno(); bus++) {
		for (devnum = 0; devnum < PCI_MAX_PCI_DEVICES-1; devnum++) {
			pci_read_config_dword(PCI_BDF(bus, devnum, 0),
					      PCI_CLASS_REVISION, &class);
			if (class >> 16 == 0xffff)
				continue;

			for (bdf = PCI_BDF(bus, devnum, 0);
					bdf <= PCI_BDF(bus, devnum,
						PCI_MAX_PCI_FUNCTIONS - 1);
					bdf += PCI_BDF(0, 0, 1)) {
				pci_read_config_dword(bdf, PCI_CLASS_REVISION,
						      &class);
				class >>= 8;
				/*
				 * Here be dragons! In case we have multiple
				 * PCI EHCI controllers, this function will
				 * be called multiple times as well. This
				 * function will scan the PCI busses, always
				 * starting from bus 0, device 0, function 0,
				 * until it finds an USB controller. The USB
				 * stack gives us an 'index' of a controller
				 * that is currently being registered, which
				 * is a number, starting from 0 and growing
				 * in ascending order as controllers are added.
				 * To avoid probing the same controller in tne
				 * subsequent runs of this function, we will
				 * skip 'index - 1' detected controllers and
				 * report the index'th controller.
				 */
				if (class != PCI_CLASS_SERIAL_USB_EHCI)
					continue;
				if (index) {
					index--;
					continue;
				}
				/* Return index'th controller. */
				return bdf;
			}
		}
	}

	return -ENODEV;
}
#endif

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **ret_hccr, struct ehci_hcor **ret_hcor)
{
	pci_dev_t pdev;
	uint32_t cmd;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;

#ifdef CONFIG_PCI_EHCI_DEVICE
	pdev = pci_find_devices(ehci_pci_ids, CONFIG_PCI_EHCI_DEVICE);
#else
	pdev = ehci_find_class(index);
#endif
	if (pdev < 0) {
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

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	/* enable busmaster */
	pci_read_config_dword(pdev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER;
	pci_write_config_dword(pdev, PCI_COMMAND, cmd);
	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}
