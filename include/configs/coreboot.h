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

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_COREBOOT
#define CONFIG_LAST_STAGE_INIT
#define CONFIG_SYS_EARLY_PCI_INIT

#define CONFIG_SYS_CAR_ADDR			0x19200000
#define CONFIG_SYS_CAR_SIZE			(16 * 1024)
#define CONFIG_SYS_MONITOR_LEN			(256 * 1024)

#define CONFIG_TRACE_EARLY_SIZE		(8 << 20)
#define CONFIG_TRACE_EARLY
#define CONFIG_TRACE_EARLY_ADDR		0x01400000

#define CONFIG_BOOTSTAGE
#define CONFIG_BOOTSTAGE_REPORT
#define CONFIG_BOOTSTAGE_FDT
#define CONFIG_CMD_BOOTSTAGE
/* Place to stash bootstage data from first-stage U-Boot */
#define CONFIG_BOOTSTAGE_STASH		0x0110f000
#define CONFIG_BOOTSTAGE_STASH_SIZE	0x7fc
#define CONFIG_BOOTSTAGE_USER_COUNT	60

#define CONFIG_SCSI_DEV_LIST		{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_NM10_AHCI},	      \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_MOBILE}, \
	{PCI_VENDOR_ID_INTEL, \
			PCI_DEVICE_ID_INTEL_COUGARPOINT_AHCI_SERIES6}, \
	{PCI_VENDOR_ID_INTEL,		\
			PCI_DEVICE_ID_INTEL_PANTHERPOINT_AHCI_MOBILE}

#define CONFIG_X86_SERIAL

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial,cbmem\0" \
					"stderr=vga,serial,cbmem\0"

#define CONFIG_CBMEM_CONSOLE

#define CONFIG_VIDEO_COREBOOT

#define CONFIG_NR_DRAM_BANKS			4

#define CONFIG_TRACE
#define CONFIG_CMD_TRACE
#define CONFIG_TRACE_BUFFER_SIZE	(16 << 20)

#define CONFIG_BOOTDELAY	2

#define CONFIG_CROS_EC
#define CONFIG_CROS_EC_LPC
#define CONFIG_CMD_CROS_EC
#define CONFIG_ARCH_EARLY_INIT_R

#endif	/* __CONFIG_H */
