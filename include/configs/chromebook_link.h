/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_CAR_ADDR			0xff7e0000
#define CONFIG_SYS_CAR_SIZE			(128 * 1024)
#define CONFIG_SYS_MONITOR_LEN			(1 << 20)
#define CONFIG_SYS_X86_START16			0xfffff800
#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_X86_RESET_VECTOR
#define CONFIG_NR_DRAM_BANKS			8

#define CONFIG_COREBOOT_SERIAL

#define CONFIG_SCSI_DEV_LIST		{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_NM10_AHCI},	      \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_MOBILE}, \
	{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_SERIES6}, \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_PANTHERPOINT_AHCI_MOBILE}

/*
 * These common x86 features are not yet supported, but are added in
 * follow-on patches in this series. Add undefs here to avoid every patch
 * having to put things back into x86-common.h
 */
#undef CONFIG_INTEL_ICH6_GPIO
#undef CONFIG_DM_GPIO
#undef CONFIG_CMD_GPIO
#undef CONFIG_VIDEO
#undef CONFIG_CFB_CONSOLE
#undef CONFIG_ICH_SPI
#undef CONFIG_SPI
#undef CONFIG_CMD_SPI
#undef CONFIG_CMD_SF
#undef CONFIG_USB_EHCI
#undef CONFIG_CMD_USB
#undef CONFIG_CMD_SCSI

#define CONFIG_PCI_MEM_BUS	0xe0000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_PREF_BUS	0xd0000000
#define CONFIG_PCI_PREF_PHYS	CONFIG_PCI_PREF_BUS
#define CONFIG_PCI_PREF_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x1000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0xefff

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial\0" \
					"stderr=vga,serial\0"

#endif	/* __CONFIG_H */
