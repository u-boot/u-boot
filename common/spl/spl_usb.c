/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived work from spl_mmc.c
 */

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <usb.h>
#include <fat.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_USB_STORAGE
static int usb_stor_curr_dev = -1; /* current device */
#endif

void spl_usb_load_image(void)
{
	int err;
	block_dev_desc_t *stor_dev;

	usb_stop();
	err = usb_init();
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		hang();
	}

#ifdef CONFIG_USB_STORAGE
	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = usb_stor_get_dev(usb_stor_curr_dev);
#endif

	debug("boot mode - FAT\n");

#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || spl_load_image_fat_os(stor_dev,
								CONFIG_SYS_USB_FAT_BOOT_PARTITION))
#endif
		err = spl_load_image_fat(stor_dev,
				CONFIG_SYS_USB_FAT_BOOT_PARTITION,
				CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME);

		if (err) {
			puts("Error loading USB device\n");
			hang();
		}
}
