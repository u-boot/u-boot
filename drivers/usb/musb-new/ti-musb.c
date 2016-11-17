/*
 * MISC driver for TI MUSB Glue.
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <linux/usb/otg.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DM_USB

static int ti_musb_wrapper_bind(struct udevice *parent)
{
	const void *fdt = gd->fdt_blob;
	int node;
	int ret;

	for (node = fdt_first_subnode(fdt, parent->of_offset); node > 0;
	     node = fdt_next_subnode(fdt, node)) {
		struct udevice *dev;
		const char *name = fdt_get_name(fdt, node, NULL);
		enum usb_dr_mode dr_mode;
		struct driver *drv;

		if (strncmp(name, "usb@", 4))
			continue;

		dr_mode = usb_get_dr_mode(node);
		switch (dr_mode) {
		case USB_DR_MODE_PERIPHERAL:
			/* Bind MUSB device */
			break;
		case USB_DR_MODE_HOST:
			/* Bind MUSB host */
			break;
		default:
			break;
		};
	}
	return 0;
}

static const struct udevice_id ti_musb_ids[] = {
	{ .compatible = "ti,am33xx-usb" },
	{ }
};

U_BOOT_DRIVER(ti_musb_wrapper) = {
	.name	= "ti-musb-wrapper",
	.id	= UCLASS_MISC,
	.of_match = ti_musb_ids,
	.bind = ti_musb_wrapper_bind,
};

#endif /* CONFIG_DM_USB */
