/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
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
#define CONFIG_ARCH_MISC_INIT

#define CONFIG_X86_SERIAL
#define CONFIG_SMSC_LPC47M

#define CONFIG_PCI_MEM_BUS		0x40000000
#define CONFIG_PCI_MEM_PHYS		CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE		0x80000000

#define CONFIG_PCI_PREF_BUS		0xc0000000
#define CONFIG_PCI_PREF_PHYS		CONFIG_PCI_PREF_BUS
#define CONFIG_PCI_PREF_SIZE		0x20000000

#define CONFIG_PCI_IO_BUS		0x2000
#define CONFIG_PCI_IO_PHYS		CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE		0xe000

#define CONFIG_SYS_EARLY_PCI_INIT
#define CONFIG_PCI_PNP
#define CONFIG_E1000

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define CONFIG_SCSI_DEV_LIST            \
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_TCF_SATA}

#define CONFIG_SPI_FLASH_SST

#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_CMD_MMC

/* Topcliff Gigabit Ethernet */
#define CONFIG_PCH_GBE
#define CONFIG_PHYLIB

/* Video is not supported */
#undef CONFIG_VIDEO
#undef CONFIG_CFB_CONSOLE

/* Environment configuration */
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0

#endif	/* __CONFIG_H */
