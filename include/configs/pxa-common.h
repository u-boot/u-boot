/*
 * Toradex Colibri PXA270 configuration file
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__CONFIG_PXA_COMMON_H__
#define	__CONFIG_PXA_COMMON_H__

#define	CONFIG_DISPLAY_CPUINFO

/*
 * KGDB
 */
#ifdef	CONFIG_CMD_KGDB
#define	CONFIG_KGDB_BAUDRATE		230400
#define	CONFIG_KGDB_SER_INDEX		2
#endif

/*
 * MMC Card Configuration
 */
#ifdef	CONFIG_CMD_MMC
#define	CONFIG_MMC
#define	CONFIG_GENERIC_MMC
#define	CONFIG_PXA_MMC_GENERIC
#define	CONFIG_CMD_FAT
#define	CONFIG_CMD_EXT2
#define	CONFIG_DOS_PARTITION
#endif

/*
 * OHCI USB
 */
#ifdef	CONFIG_CMD_USB
#define	CONFIG_USB_OHCI_NEW
#define	CONFIG_SYS_USB_OHCI_CPU_INIT
#define	CONFIG_SYS_USB_OHCI_BOARD_INIT
#define	CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define	CONFIG_SYS_USB_OHCI_REGS_BASE		0x4c000000
#define	CONFIG_SYS_USB_OHCI_SLOT_NAME		"pxa-ohci"
#define	CONFIG_USB_STORAGE
#endif

#endif	/* __CONFIG_PXA_COMMON_H__ */
