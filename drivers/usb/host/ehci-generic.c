/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <reset.h>
#include <asm/io.h>
#include <dm.h>
#include "ehci.h"

/*
 * Even though here we don't explicitly use "struct ehci_ctrl"
 * ehci_register() expects it to be the first thing that resides in
 * device's private data.
 */
struct generic_ehci {
	struct ehci_ctrl ctrl;
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int i;

	for (i = 0; ; i++) {
		struct clk clk;
		int ret;

		ret = clk_get_by_index(dev, i, &clk);
		if (ret < 0)
			break;
		if (clk_enable(&clk))
			printf("failed to enable clock %d\n", i);
		clk_free(&clk);
	}

	for (i = 0; ; i++) {
		struct reset_ctl reset;
		int ret;

		ret = reset_get_by_index(dev, i, &reset);
		if (ret < 0)
			break;
		if (reset_deassert(&reset))
			printf("failed to deassert reset %d\n", i);
		reset_free(&reset);
	}

	hccr = map_physmem(devfdt_get_addr(dev), 0x100, MAP_NOCACHE);
	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
				    HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "generic-ehci" },
	{ }
};

U_BOOT_DRIVER(ehci_generic) = {
	.name	= "ehci_generic",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.probe = ehci_usb_probe,
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct generic_ehci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
