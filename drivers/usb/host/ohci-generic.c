/*
 * Copyright (C) 2015 Alexey Brodkin <abrodkin@synopsys.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/ofnode.h>
#include "ohci.h"

#if !defined(CONFIG_USB_OHCI_NEW)
# error "Generic OHCI driver requires CONFIG_USB_OHCI_NEW"
#endif

struct generic_ohci {
	ohci_t ohci;
	struct clk *clocks;	/* clock list */
	int clock_count;	/* number of clock in clock list */
};

static int ohci_usb_probe(struct udevice *dev)
{
	struct ohci_regs *regs = (struct ohci_regs *)devfdt_get_addr(dev);
	struct generic_ohci *priv = dev_get_priv(dev);
	int i, err, ret, clock_nb;

	err = 0;
	priv->clock_count = 0;
	clock_nb = dev_count_phandle_with_args(dev, "clocks", "#clock-cells");
	if (clock_nb > 0) {
		priv->clocks = devm_kcalloc(dev, clock_nb, sizeof(struct clk),
					    GFP_KERNEL);
		if (!priv->clocks)
			return -ENOMEM;

		for (i = 0; i < clock_nb; i++) {
			err = clk_get_by_index(dev, i, &priv->clocks[i]);
			if (err < 0)
				break;

			err = clk_enable(&priv->clocks[i]);
			if (err) {
				error("failed to enable clock %d\n", i);
				clk_free(&priv->clocks[i]);
				goto clk_err;
			}
			priv->clock_count++;
		}
	} else if (clock_nb != -ENOENT) {
		error("failed to get clock phandle(%d)\n", clock_nb);
		return clock_nb;
	}

	err = ohci_register(dev, regs);
	if (err)
		goto clk_err;

	return 0;

clk_err:
	ret = clk_release_all(priv->clocks, priv->clock_count);
	if (ret)
		error("failed to disable all clocks\n");

	return err;
}

static int ohci_usb_remove(struct udevice *dev)
{
	struct generic_ohci *priv = dev_get_priv(dev);
	int ret;

	ret = ohci_deregister(dev);
	if (ret)
		return ret;

	return clk_release_all(priv->clocks, priv->clock_count);
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
