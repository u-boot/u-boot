// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#include <common.h>
#include <env.h>
#include <errno.h>
#include <image.h>
#include <log.h>
#include <spl.h>
#include <net.h>
#include <linux/libfdt.h>

#if defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
static ulong spl_net_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, count, (ulong)buf);
	memcpy(buf, (void *)(image_load_addr + sector), count);
	return count;
}

static int spl_net_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	struct spl_load_info load = {
		.bl_len = 1,
		.read = spl_net_load_read,
	};
	int rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	if (rv == 0) {
		printf("No Ethernet devices found\n");
		return -ENODEV;
	}
	if (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	if (rv < 0) {
		printf("Problem booting with BOOTP\n");
		return rv;
	}

	return spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
int spl_net_load_image_cpgmac(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	return spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
int spl_net_load_image_usb(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#if CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	return spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
