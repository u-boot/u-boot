/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived work from spl_usb.c
 */

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <sata.h>
#include <fat.h>
#include <version.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

void spl_sata_load_image(void)
{
	int err;
	block_dev_desc_t *stor_dev;

	err = init_sata(CONFIG_SPL_SATA_BOOT_DEVICE);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: sata init failed: err - %d\n", err);
#endif
		hang();
	} else {
		/* try to recognize storage devices immediately */
		stor_dev = scsi_get_dev(0);
	}

#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || spl_load_image_fat_os(stor_dev,
									CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	err = spl_load_image_fat(stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME);
	if (err) {
		puts("Error loading sata device\n");
		hang();
	}
}
