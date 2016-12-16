/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <spl.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_ETH_SUPPORT) || defined(CONFIG_SPL_USBETH_SUPPORT)
static int spl_net_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int rv;

	env_init();
	env_relocate();
	setenv("autoload", "yes");
	load_addr = CONFIG_SYS_TEXT_BASE - sizeof(struct image_header);
	rv = eth_initialize();
	if (rv == 0) {
		printf("No Ethernet devices found\n");
		return -ENODEV;
	}
	if (bootdev->boot_device_name)
		setenv("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	if (rv < 0) {
		printf("Problem booting with BOOTP\n");
		return rv;
	}
	return spl_parse_image_header(spl_image,
				      (struct image_header *)load_addr);
}
#endif

#ifdef CONFIG_SPL_ETH_SUPPORT
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

#ifdef CONFIG_SPL_USBETH_SUPPORT
int spl_net_load_image_usb(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";

	return spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
