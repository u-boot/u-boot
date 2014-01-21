/*
 *  Copyright (C) 2013 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/usb/ch9.h>

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	if (!strcmp(name, "usb_dnl_thor")) {
		put_unaligned(CONFIG_G_DNL_THOR_VENDOR_NUM, &dev->idVendor);
		put_unaligned(CONFIG_G_DNL_THOR_PRODUCT_NUM, &dev->idProduct);
	} else {
		put_unaligned(CONFIG_G_DNL_VENDOR_NUM, &dev->idVendor);
		put_unaligned(CONFIG_G_DNL_PRODUCT_NUM, &dev->idProduct);
	}
	return 0;
}
