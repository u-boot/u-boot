/*
 * Copyright (C) 2015 Google, Inc
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
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_ARCH_MISC_INIT

#define CONFIG_SMSC_LPC47M

#define CONFIG_PCI_PNP
#define CONFIG_RTL8169
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial\0" \
					"stderr=vga,serial\0"

#define CONFIG_SCSI_DEV_LIST		\
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SATA}, \
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SATA_ALT}


#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_CMD_MMC

#undef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT		1

#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO

#define CONFIG_FIT_SIGNATURE
#define CONFIG_RSA

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x007fe000

#endif	/* __CONFIG_H */
