/**
 * dwc3-generic.c - Generic DWC3 Glue layer
 *
 * Copyright (C) 2016 Xilinx, Inc.
 *
 * Based on dwc3-omap.c.
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/usb/otg.h>
#include <linux/compat.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <malloc.h>
#include <usb.h>
#include "core.h"
#include "gadget.h"
#include "linux-compat.h"

DECLARE_GLOBAL_DATA_PTR;

int usb_gadget_handle_interrupts(int index)
{
	struct dwc3 *priv;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device(UCLASS_USB_DEV_GENERIC, &dev);
	if (!dev || ret) {
		error("No USB device found\n");
		return -ENODEV;
	}

	priv = dev_get_priv(dev);

	dwc3_gadget_uboot_handle_interrupt(priv);

	return 0;
}

static int dwc3_generic_peripheral_probe(struct udevice *dev)
{
	struct dwc3 *priv = dev_get_priv(dev);

	return dwc3_init(priv);
}

static int dwc3_generic_peripheral_remove(struct udevice *dev)
{
	struct dwc3 *priv = dev_get_priv(dev);

	dwc3_remove(priv);

	return 0;
}

static int dwc3_generic_peripheral_ofdata_to_platdata(struct udevice *dev)
{
	struct dwc3 *priv = dev_get_priv(dev);
	int node = dev->of_offset;

	priv->regs = (void *)dev_get_addr(dev);
	priv->regs += DWC3_GLOBALS_REGS_START;

	priv->maximum_speed = usb_get_maximum_speed(node);
	if (priv->maximum_speed == USB_SPEED_UNKNOWN) {
		error("Invalid usb maximum speed\n");
		return -ENODEV;
	}

	priv->dr_mode = usb_get_dr_mode(node);
	if (priv->dr_mode == USB_DR_MODE_UNKNOWN) {
		error("Invalid usb mode setup\n");
		return -ENODEV;
	}

	return 0;
}

static int dwc3_generic_peripheral_bind(struct udevice *dev)
{
	return device_probe(dev);
}

U_BOOT_DRIVER(dwc3_generic_peripheral) = {
	.name	= "dwc3-generic-peripheral",
	.id	= UCLASS_USB_DEV_GENERIC,
	.ofdata_to_platdata = dwc3_generic_peripheral_ofdata_to_platdata,
	.probe = dwc3_generic_peripheral_probe,
	.remove = dwc3_generic_peripheral_remove,
	.bind = dwc3_generic_peripheral_bind,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct dwc3),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static int dwc3_generic_bind(struct udevice *parent)
{
	const void *fdt = gd->fdt_blob;
	int node;
	int ret;

	for (node = fdt_first_subnode(fdt, parent->of_offset); node > 0;
	     node = fdt_next_subnode(fdt, node)) {
		const char *name = fdt_get_name(fdt, node, NULL);
		enum usb_dr_mode dr_mode;
		struct udevice *dev;

		debug("%s: subnode name: %s\n", __func__, name);
		if (strncmp(name, "dwc3@", 4))
			continue;

		dr_mode = usb_get_dr_mode(node);

		switch (dr_mode) {
		case USB_DR_MODE_PERIPHERAL:
		case USB_DR_MODE_OTG:
			debug("%s: dr_mode: OTG or Peripheral\n", __func__);
			ret = device_bind_driver_to_node(parent,
							 "dwc3-generic-peripheral",
							 name, node, &dev);
			if (ret) {
				debug("%s: not able to bind usb device mode\n",
				      __func__);
				return ret;
			}
			break;
		case USB_DR_MODE_HOST:
			debug("%s: dr_mode: HOST\n", __func__);
			ret = device_bind_driver_to_node(parent,
							 "dwc3-generic-host",
							 name, node, &dev);
			if (ret) {
				debug("%s: not able to bind usb host mode\n",
				      __func__);
				return ret;
			}
			break;
		default:
			break;
		};
	}

	return 0;
}

static const struct udevice_id dwc3_generic_ids[] = {
	{ .compatible = "xlnx,zynqmp-dwc3" },
	{ }
};

U_BOOT_DRIVER(dwc3_generic_wrapper) = {
	.name	= "dwc3-generic-wrapper",
	.id	= UCLASS_MISC,
	.of_match = dwc3_generic_ids,
	.bind = dwc3_generic_bind,
};
