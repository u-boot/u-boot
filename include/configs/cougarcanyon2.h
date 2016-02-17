/*
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(2 << 20)
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SMSC_SIO1007

#define CONFIG_PCI_PNP

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial,i8042-kbd,usbkbd\0" \
					"stdout=serial,vga\0" \
					"stderr=serial,vga\0"

#define CONFIG_SCSI_DEV_LIST		\
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_PANTHERPOINT_AHCI_MOBILE}

/* Environment configuration */
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x5ff000

/* Video is not supported for now */
#undef CONFIG_VIDEO
#undef CONFIG_CFB_CONSOLE

#endif	/* __CONFIG_H */
