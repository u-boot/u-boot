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
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_X86_SERIAL
#define CONFIG_SMSC_LPC47M

#define CONFIG_PCI_MEM_BUS		0xd0000000
#define CONFIG_PCI_MEM_PHYS		CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE		0x10000000

#define CONFIG_PCI_PREF_BUS		0xc0000000
#define CONFIG_PCI_PREF_PHYS		CONFIG_PCI_PREF_BUS
#define CONFIG_PCI_PREF_SIZE		0x10000000

#define CONFIG_PCI_IO_BUS		0x2000
#define CONFIG_PCI_IO_PHYS		CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE		0xe000

#define CONFIG_SYS_EARLY_PCI_INIT
#define CONFIG_PCI_PNP
#define CONFIG_RTL8169
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial\0" \
					"stderr=vga,serial\0"

#define CONFIG_SCSI_DEV_LIST            \
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SATA}
#define CONFIG_SPI_FLASH_SST

#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_CMD_MMC

#undef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT		1

#define CONFIG_X86_OPTION_ROM_FILE		vga.bin
#define CONFIG_X86_OPTION_ROM_ADDR		0xfff90000

#ifndef CONFIG_SYS_COREBOOT
#define CONFIG_VIDEO_VESA
#endif
#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO
#define CONFIG_VGA_AS_SINGLE_DEVICE

#define CONFIG_FIT_SIGNATURE
#define CONFIG_RSA

/* Avoid a warning in the Realtek Ethernet driver */
#define CONFIG_SYS_CACHELINE_SIZE 16

#endif	/* __CONFIG_H */
