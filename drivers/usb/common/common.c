/*
 * Provides code common for host and device side USB.
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <linux/usb/otg.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *const usb_dr_modes[] = {
	[USB_DR_MODE_UNKNOWN]		= "",
	[USB_DR_MODE_HOST]		= "host",
	[USB_DR_MODE_PERIPHERAL]	= "peripheral",
	[USB_DR_MODE_OTG]		= "otg",
};

enum usb_dr_mode usb_get_dr_mode(int node)
{
	const void *fdt = gd->fdt_blob;
	const char *dr_mode;
	int i;

	dr_mode = fdt_getprop(fdt, node, "dr_mode", NULL);
	if (!dr_mode) {
		pr_err("usb dr_mode not found\n");
		return USB_DR_MODE_UNKNOWN;
	}

	for (i = 0; i < ARRAY_SIZE(usb_dr_modes); i++)
		if (!strcmp(dr_mode, usb_dr_modes[i]))
			return i;

	return USB_DR_MODE_UNKNOWN;
}
