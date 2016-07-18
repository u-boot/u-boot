/*
 * Copyright (c) 2015, Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>
#include <usb.h>

#include "xhci.h"

#ifndef CONFIG_DM_USB

/*
 * Create the appropriate control structures to manage a new XHCI host
 * controller.
 */
int xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
		  struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	pci_dev_t pdev;
	uint32_t cmd;
	int len;

	pdev = pci_find_class(PCI_CLASS_SERIAL_USB_XHCI, index);
	if (pdev < 0) {
		printf("XHCI host controller not found\n");
		return -1;
	}

	hccr = (struct xhci_hccr *)pci_map_bar(pdev,
			PCI_BASE_ADDRESS_0, PCI_REGION_MEM);
	len = HC_LENGTH(xhci_readl(&hccr->cr_capbase));
	hcor = (struct xhci_hcor *)((uint32_t)hccr + len);

	debug("XHCI-PCI init hccr 0x%x and hcor 0x%x hc_length %d\n",
	      (uint32_t)hccr, (uint32_t)hcor, len);

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	/* enable busmaster */
	pci_read_config_dword(pdev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER;
	pci_write_config_dword(pdev, PCI_COMMAND, cmd);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding * to the XHCI host
 * controller
 */
void xhci_hcd_stop(int index)
{
}

#else

struct xhci_pci_priv {
	struct xhci_ctrl ctrl;	/* Needs to come first in this struct! */
};

static void xhci_pci_init(struct udevice *dev, struct xhci_hccr **ret_hccr,
			  struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	u32 cmd;

	hccr = (struct xhci_hccr *)dm_pci_map_bar(dev,
			PCI_BASE_ADDRESS_0, PCI_REGION_MEM);
	hcor = (struct xhci_hcor *)((uintptr_t) hccr +
			HC_LENGTH(xhci_readl(&hccr->cr_capbase)));

	debug("XHCI-PCI init hccr 0x%x and hcor 0x%x hc_length %d\n",
	      (u32)hccr, (u32)hcor,
	      (u32)HC_LENGTH(xhci_readl(&hccr->cr_capbase)));

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	/* enable busmaster */
	dm_pci_read_config32(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER;
	dm_pci_write_config32(dev, PCI_COMMAND, cmd);
}

static int xhci_pci_probe(struct udevice *dev)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;

	xhci_pci_init(dev, &hccr, &hcor);

	return xhci_register(dev, hccr, hcor);
}

static int xhci_pci_remove(struct udevice *dev)
{
	int ret;

	ret = xhci_deregister(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id xhci_pci_ids[] = {
	{ .compatible = "xhci-pci" },
	{ }
};

U_BOOT_DRIVER(xhci_pci) = {
	.name	= "xhci_pci",
	.id	= UCLASS_USB,
	.probe = xhci_pci_probe,
	.remove = xhci_pci_remove,
	.of_match = xhci_pci_ids,
	.ops	= &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct xhci_pci_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static struct pci_device_id xhci_pci_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_SERIAL_USB_XHCI, ~0) },
	{},
};

U_BOOT_PCI_DEVICE(xhci_pci, xhci_pci_supported);

#endif /* CONFIG_DM_USB */
