/*
 * g_dnl.c -- USB Downloader Gadget
 *
 * Copyright (C) 2012 Samsung Electronics
 * Lukasz Majewski  <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>

#include <mmc.h>
#include <part.h>

#include <g_dnl.h>
#include <usb_mass_storage.h>
#include <dfu.h>

#include "gadget_chips.h"
#include "composite.c"

/*
 * One needs to define the following:
 * CONFIG_G_DNL_VENDOR_NUM
 * CONFIG_G_DNL_PRODUCT_NUM
 * CONFIG_G_DNL_MANUFACTURER
 * at e.g. ./include/configs/<board>.h
 */

#define STRING_MANUFACTURER 25
#define STRING_PRODUCT 2
/* Index of String Descriptor describing this configuration */
#define STRING_USBDOWN 2
/* Number of supported configurations */
#define CONFIGURATION_NUMBER 1

#define DRIVER_VERSION		"usb_dnl 2.0"

static const char shortname[] = "usb_dnl_";
static const char product[] = "USB download gadget";
static const char manufacturer[] = CONFIG_G_DNL_MANUFACTURER;

static struct usb_device_descriptor device_desc = {
	.bLength = sizeof device_desc,
	.bDescriptorType = USB_DT_DEVICE,

	.bcdUSB = __constant_cpu_to_le16(0x0200),
	.bDeviceClass = USB_CLASS_COMM,
	.bDeviceSubClass = 0x02, /*0x02:CDC-modem , 0x00:CDC-serial*/

	.idVendor = __constant_cpu_to_le16(CONFIG_G_DNL_VENDOR_NUM),
	.idProduct = __constant_cpu_to_le16(CONFIG_G_DNL_PRODUCT_NUM),
	.iProduct = STRING_PRODUCT,
	.bNumConfigurations = 1,
};

/*
 * static strings, in UTF-8
 * IDs for those strings are assigned dynamically at g_dnl_bind()
 */
static struct usb_string g_dnl_string_defs[] = {
	{.s = manufacturer},
	{.s = product},
	{ }		/* end of list */
};

static struct usb_gadget_strings g_dnl_string_tab = {
	.language = 0x0409, /* en-us */
	.strings = g_dnl_string_defs,
};

static struct usb_gadget_strings *g_dnl_composite_strings[] = {
	&g_dnl_string_tab,
	NULL,
};

static int g_dnl_unbind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;

	debug("%s: calling usb_gadget_disconnect for "
			"controller '%s'\n", shortname, gadget->name);
	usb_gadget_disconnect(gadget);

	return 0;
}

static int g_dnl_do_config(struct usb_configuration *c)
{
	const char *s = c->cdev->driver->name;
	int ret = -1;

	debug("%s: configuration: 0x%p composite dev: 0x%p\n",
	      __func__, c, c->cdev);

	printf("GADGET DRIVER: %s\n", s);
	if (!strcmp(s, "usb_dnl_dfu"))
		ret = dfu_add(c);
	else if (!strcmp(s, "usb_dnl_ums"))
		ret = fsg_add(c);

	return ret;
}

static int g_dnl_config_register(struct usb_composite_dev *cdev)
{
	static struct usb_configuration config = {
		.label = "usb_dnload",
		.bmAttributes =	USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
		.bConfigurationValue =	CONFIGURATION_NUMBER,
		.iConfiguration =	STRING_USBDOWN,

		.bind = g_dnl_do_config,
	};

	return usb_add_config(cdev, &config);
}

__weak
int g_dnl_bind_fixup(struct usb_device_descriptor *dev)
{
	return 0;
}

static int g_dnl_bind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	int id, ret;
	int gcnum;

	debug("%s: gadget: 0x%p cdev: 0x%p\n", __func__, gadget, cdev);

	id = usb_string_id(cdev);

	if (id < 0)
		return id;
	g_dnl_string_defs[0].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;

	g_dnl_string_defs[1].id = id;
	device_desc.iProduct = id;

	g_dnl_bind_fixup(&device_desc);
	ret = g_dnl_config_register(cdev);
	if (ret)
		goto error;

	gcnum = usb_gadget_controller_number(gadget);

	debug("gcnum: %d\n", gcnum);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		debug("%s: controller '%s' not recognized\n",
			shortname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	debug("%s: calling usb_gadget_connect for "
			"controller '%s'\n", shortname, gadget->name);
	usb_gadget_connect(gadget);

	return 0;

 error:
	g_dnl_unbind(cdev);
	return -ENOMEM;
}

static struct usb_composite_driver g_dnl_driver = {
	.name = NULL,
	.dev = &device_desc,
	.strings = g_dnl_composite_strings,

	.bind = g_dnl_bind,
	.unbind = g_dnl_unbind,
};

int g_dnl_register(const char *type)
{
	/* We only allow "dfu" atm, so 3 should be enough */
	static char name[sizeof(shortname) + 3];
	int ret;

	if (!strcmp(type, "dfu")) {
		strcpy(name, shortname);
		strcat(name, type);
	} else if (!strcmp(type, "ums")) {
		strcpy(name, shortname);
		strcat(name, type);
	} else {
		printf("%s: unknown command: %s\n", __func__, type);
		return -EINVAL;
	}

	g_dnl_driver.name = name;

	debug("%s: g_dnl_driver.name: %s\n", __func__, g_dnl_driver.name);
	ret = usb_composite_register(&g_dnl_driver);

	if (ret) {
		printf("%s: failed!, error: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

void g_dnl_unregister(void)
{
	usb_composite_unregister(&g_dnl_driver);
}
