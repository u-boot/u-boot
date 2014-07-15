/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>

#include "usb_ether.h"

typedef void (*usb_eth_before_probe)(void);
typedef int (*usb_eth_probe)(struct usb_device *dev, unsigned int ifnum,
			struct ueth_data *ss);
typedef int (*usb_eth_get_info)(struct usb_device *dev, struct ueth_data *ss,
			struct eth_device *dev_desc);

struct usb_eth_prob_dev {
	usb_eth_before_probe	before_probe; /* optional */
	usb_eth_probe			probe;
	usb_eth_get_info		get_info;
};

/* driver functions go here, each bracketed by #ifdef CONFIG_USB_ETHER_xxx */
static const struct usb_eth_prob_dev prob_dev[] = {
#ifdef CONFIG_USB_ETHER_ASIX
	{
		.before_probe = asix_eth_before_probe,
		.probe = asix_eth_probe,
		.get_info = asix_eth_get_info,
	},
#endif
#ifdef CONFIG_USB_ETHER_MCS7830
	{
		.before_probe = mcs7830_eth_before_probe,
		.probe = mcs7830_eth_probe,
		.get_info = mcs7830_eth_get_info,
	},
#endif
#ifdef CONFIG_USB_ETHER_SMSC95XX
	{
		.before_probe = smsc95xx_eth_before_probe,
		.probe = smsc95xx_eth_probe,
		.get_info = smsc95xx_eth_get_info,
	},
#endif
	{ },		/* END */
};

static int usb_max_eth_dev; /* number of highest available usb eth device */
static struct ueth_data usb_eth[USB_MAX_ETH_DEV];

/*******************************************************************************
 * tell if current ethernet device is a usb dongle
 */
int is_eth_dev_on_usb_host(void)
{
	int i;
	struct eth_device *dev = eth_get_dev();

	if (dev) {
		for (i = 0; i < usb_max_eth_dev; i++)
			if (&usb_eth[i].eth_dev == dev)
				return 1;
	}
	return 0;
}

/*
 * Given a USB device, ask each driver if it can support it, and attach it
 * to the first driver that says 'yes'
 */
static void probe_valid_drivers(struct usb_device *dev)
{
	struct eth_device *eth;
	int j;

	for (j = 0; prob_dev[j].probe && prob_dev[j].get_info; j++) {
		if (!prob_dev[j].probe(dev, 0, &usb_eth[usb_max_eth_dev]))
			continue;
		/*
		 * ok, it is a supported eth device. Get info and fill it in
		 */
		eth = &usb_eth[usb_max_eth_dev].eth_dev;
		if (prob_dev[j].get_info(dev,
			&usb_eth[usb_max_eth_dev],
			eth)) {
			/* found proper driver */
			/* register with networking stack */
			usb_max_eth_dev++;

			/*
			 * usb_max_eth_dev must be incremented prior to this
			 * call since eth_current_changed (internally called)
			 * relies on it
			 */
			eth_register(eth);
			if (eth_write_hwaddr(eth, "usbeth",
					usb_max_eth_dev - 1))
				puts("Warning: failed to set MAC address\n");
			break;
			}
		}
	}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
int usb_host_eth_scan(int mode)
{
	int i, old_async;
	struct usb_device *dev;


	if (mode == 1)
		printf("       scanning usb for ethernet devices... ");

	old_async = usb_disable_asynch(1); /* asynch transfer not allowed */

	/* unregister a previously detected device */
	for (i = 0; i < usb_max_eth_dev; i++)
		eth_unregister(&usb_eth[i].eth_dev);

	memset(usb_eth, 0, sizeof(usb_eth));

	for (i = 0; prob_dev[i].probe; i++) {
		if (prob_dev[i].before_probe)
			prob_dev[i].before_probe();
	}

	usb_max_eth_dev = 0;
	for (i = 0; i < USB_MAX_DEVICE; i++) {
		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		if (dev == NULL)
			break; /* no more devices available */

		/* find valid usb_ether driver for this device, if any */
		probe_valid_drivers(dev);

		/* check limit */
		if (usb_max_eth_dev == USB_MAX_ETH_DEV) {
			printf("max USB Ethernet Device reached: %d stopping\n",
				usb_max_eth_dev);
			break;
		}
	} /* for */

	usb_disable_asynch(old_async); /* restore asynch value */
	printf("%d Ethernet Device(s) found\n", usb_max_eth_dev);
	if (usb_max_eth_dev > 0)
		return 0;
	return -1;
}
