/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Tuomas Tynkkynen
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* Physical memory map */

#define CONFIG_SYS_SDRAM_BASE		0x40000000

/* GUIDs for capsule updatable firmware images */
#define QEMU_ARM_UBOOT_IMAGE_GUID \
	EFI_GUID(0xf885b085, 0x99f8, 0x45af, 0x84, 0x7d, \
		 0xd5, 0x14, 0x10, 0x7a, 0x4a, 0x2c)

#define QEMU_ARM64_UBOOT_IMAGE_GUID \
	EFI_GUID(0x058b7d83, 0x50d5, 0x4c47, 0xa1, 0x95, \
		 0x60, 0xd8, 0x6a, 0xd3, 0x41, 0xc4)

/* Try files from QEMU's -kernel/-initrd, through the QEMU firmware device. */
#define BOOTENV_DEV_QFW(devtypeu, devtypel, instance) \
	"bootcmd_qfw= " \
		"if qfw load $kernel_addr_r $ramdisk_addr_r; then " \
		"  booti $kernel_addr_r $ramdisk_addr_r:$filesize $fdtcontroladdr; " \
		"  if test $? -eq 1; then " \
		"    bootz $kernel_addr_r $ramdisk_addr_r:$filesize $fdtcontroladdr; " \
		"  fi ; " \
		"fi\0"
#define BOOTENV_DEV_NAME_QFW(devtypeu, devtypel, instance) "qfw "

/* For timer, QEMU emulates an ARMv7/ARMv8 architected timer */

/* Environment options */

#if CONFIG_IS_ENABLED(CMD_USB)
# define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
# define BOOT_TARGET_USB(func)
#endif

#if CONFIG_IS_ENABLED(CMD_SCSI)
# define BOOT_TARGET_SCSI(func) func(SCSI, scsi, 0)
#else
# define BOOT_TARGET_SCSI(func)
#endif

#if CONFIG_IS_ENABLED(CMD_VIRTIO)
# define BOOT_TARGET_VIRTIO(func) func(VIRTIO, virtio, 0)
#else
# define BOOT_TARGET_VIRTIO(func)
#endif

#if CONFIG_IS_ENABLED(CMD_NVME)
# define BOOT_TARGET_NVME(func) func(NVME, nvme, 0)
#else
# define BOOT_TARGET_NVME(func)
#endif

#if CONFIG_IS_ENABLED(CMD_DHCP)
# define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	func(QFW, qfw, na) \
	BOOT_TARGET_USB(func) \
	BOOT_TARGET_SCSI(func) \
	BOOT_TARGET_VIRTIO(func) \
	BOOT_TARGET_NVME(func) \
	BOOT_TARGET_DHCP(func)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr=0x40000000\0" \
	"scriptaddr=0x40200000\0" \
	"pxefile_addr_r=0x40300000\0" \
	"kernel_addr_r=0x40400000\0" \
	"ramdisk_addr_r=0x44000000\0" \
	BOOTENV

#endif /* __CONFIG_H */
