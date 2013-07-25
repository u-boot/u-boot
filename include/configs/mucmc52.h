/*
 * (C) Copyright 2008-2009
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define	CONFIG_MUCMC52		1	/* MUCMC52 board	*/
#define	CONFIG_HOSTNAME		mucmc52

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF00000
#endif

#include "manroland/common.h"
#include "manroland/mpc5200-common.h"

#define	CONFIG_LAST_STAGE_INIT
/*
 * Serial console configuration
 */
#define	CONFIG_BAUDRATE		38400	/* ... at 38400 bps	*/

#define	CONFIG_CMD_PCI

/*
 * Flash configuration
 */
#define	CONFIG_SYS_MAX_FLASH_SECT	67

/*
 * Environment settings
 */
#define	CONFIG_ENV_SECT_SIZE	0x20000

/*
 * Memory map
 */
#define	CONFIG_SYS_STATUS1_BASE	0x80600200
#define	CONFIG_SYS_STATUS2_BASE	0x80600300
#define	CONFIG_SYS_PMI_UNI_BASE	0x80800000
#define	CONFIG_SYS_PMI_BROAD_BASE	0x80810000

/*
 * GPIO configuration
 */
#define	CONFIG_SYS_GPS_PORT_CONFIG	0x8D550644

#define	CONFIG_SYS_MEMTEST_START	0x00100000
#define	CONFIG_SYS_MEMTEST_END		0x00f00000

#define	CONFIG_SYS_LOAD_ADDR		0x100000

#define	CONFIG_SYS_BOOTCS_CFG		0x0004FB00

/* 8Mbit SRAM @0x80100000 */
#define	CONFIG_SYS_CS1_SIZE		0x00100000
#define	CONFIG_SYS_CS1_CFG		0x00019B00

#define CONFIG_SYS_SRAM_SIZE		CONFIG_SYS_CS1_SIZE

/* FRAM 32Kbyte @0x80700000 */
#define	CONFIG_SYS_CS2_START		0x80700000
#define	CONFIG_SYS_CS2_SIZE		0x00008000
#define	CONFIG_SYS_CS2_CFG		0x00019800

/* Display H1, Status Inputs, EPLD @0x80600000 */
#define	CONFIG_SYS_CS3_START		0x80600000
#define	CONFIG_SYS_CS3_SIZE		0x00100000
#define	CONFIG_SYS_CS3_CFG		0x00019800

/* PMI Unicast 32Kbyte @0x80800000 */
#define	CONFIG_SYS_CS6_START		CONFIG_SYS_PMI_UNI_BASE
#define	CONFIG_SYS_CS6_SIZE		0x00008000
#define	CONFIG_SYS_CS6_CFG		0xFFFFF930

/* PMI Broadcast 32Kbyte @0x80810000 */
#define	CONFIG_SYS_CS7_START		CONFIG_SYS_PMI_BROAD_BASE
#define	CONFIG_SYS_CS7_SIZE		0x00008000
#define	CONFIG_SYS_CS7_CFG		0xFF00F930

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */
#define	CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 2 drives per IDE bus	*/

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define	CONFIG_PCI		1
#define	CONFIG_PCI_PNP		1
#define	CONFIG_PCI_SCAN_SHOW	1
#define	CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define	CONFIG_PCI_MEM_BUS	0x40000000
#define	CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define	CONFIG_PCI_MEM_SIZE	0x10000000

#define	CONFIG_PCI_IO_BUS	0x50000000
#define	CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define	CONFIG_PCI_IO_SIZE	0x01000000

#define	CONFIG_SYS_ISA_IO		CONFIG_PCI_IO_BUS

/*---------------------------------------------------------------------*/
/* Display addresses						       */
/*---------------------------------------------------------------------*/

#define	CONFIG_SYS_DISP_CHR_RAM	(CONFIG_SYS_DISPLAY_BASE + 0x38)
#define	CONFIG_SYS_DISP_CWORD		(CONFIG_SYS_DISPLAY_BASE + 0x30)

#endif /* __CONFIG_H */
