/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_ARCH_MISC_INIT

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define CONFIG_SCSI_DEV_LIST		\
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SATA}, \
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SATA_ALT}

#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SDMA

#undef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT		1

#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO
#define CONFIG_CMD_BMP

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x006ef000

#undef CONFIG_BOOTARGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_BOOTARGS		\
	"root=/dev/sda2 ro quiet"
#define CONFIG_BOOTCOMMAND	\
	"load scsi 0:2 03000000 /boot/vmlinuz-${kernel-ver}-generic;"	\
	"load scsi 0:2 04000000 /boot/initrd.img-${kernel-ver}-generic;" \
	"run boot"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS				\
	"kernel-ver=4.4.0-22\0"					\
	"boot=zboot 03000000 0 04000000 ${filesize}\0"		\
	"upd_uboot=tftp 100000 conga/u-boot.rom;"		\
		"sf probe;sf update 100000 0 800000;saveenv\0"

#define CONFIG_PREBOOT

#endif	/* __CONFIG_H */
