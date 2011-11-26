/*
 * Toradex Colibri PXA270 configuration file
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
