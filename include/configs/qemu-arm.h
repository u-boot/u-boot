/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Tuomas Tynkkynen
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* Physical memory map */

#define CFG_SYS_SDRAM_BASE		0x40000000

/* GUIDs for capsule updatable firmware images */
#define QEMU_ARM_UBOOT_IMAGE_GUID \
	EFI_GUID(0xf885b085, 0x99f8, 0x45af, 0x84, 0x7d, \
		 0xd5, 0x14, 0x10, 0x7a, 0x4a, 0x2c)

#define QEMU_ARM64_UBOOT_IMAGE_GUID \
	EFI_GUID(0x058b7d83, 0x50d5, 0x4c47, 0xa1, 0x95, \
		 0x60, 0xd8, 0x6a, 0xd3, 0x41, 0xc4)

/* For timer, QEMU emulates an ARMv7/ARMv8 architected timer */

/* Environment options */
#define BOOT_TARGETS	"qfw usb scsi virtio nvme dhcp"

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr=0x40000000\0" \
	"scriptaddr=0x40200000\0" \
	"pxefile_addr_r=0x40300000\0" \
	"kernel_addr_r=0x40400000\0" \
	"ramdisk_addr_r=0x44000000\0" \
	"boot_targets=" BOOT_TARGETS "\0"

#endif /* __CONFIG_H */
