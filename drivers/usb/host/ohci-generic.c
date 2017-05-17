/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include "ohci.h"

#if !defined(CONFIG_USB_OHCI_NEW)
# error "Generic OHCI driver requires CONFIG_USB_OHCI_NEW"
#endif

struct generic_ohci {
	ohci_t ohci;
};

static int ohci_usb_probe(struct udevice *dev)
{
	struct ohci_regs *regs = (struct ohci_regs *)devfdt_get_addr(dev);

	return ohci_register(dev, regs);
}

static int ohci_usb_remove(struct udevice *dev)
{
	return ohci_deregister(dev);
}

static const struct udevice_id ohci_usb_ids[] = {
	{ .compatible = "generic-ohci" },
	{ }
};

U_BOOT_DRIVER(ohci_generic) = {
	.name	= "ohci_generic",
	.id	= UCLASS_USB,
	.of_match = ohci_usb_ids,
	.probe = ohci_usb_probe,
	.remove = ohci_usb_remove,
	.ops	= &ohci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct generic_ohci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
