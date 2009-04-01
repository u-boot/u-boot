/*
 * (C) Copyright 2004
 * IMMS, gGmbH <www.imms.de>
 * Thomas Elste <info@elste.org>
 *
 * Configuation settings for ModNET50 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM7		1	/* This is a ARM7 CPU	*/
#define CONFIG_ARM_THUMB	1	/* this is an ARM720TDMI */
#define CONFIG_NETARM                   /* it's a Netsiclicon NET+ARM */
#undef  CONFIG_NETARM_NET40_REV2        /* it's a Net+40 Rev. 2 */
#undef  CONFIG_NETARM_NET40_REV4	/* it's a Net+40 Rev. 4 */
#define CONFIG_NETARM_NET50             /* it's a Net+50 */

#define CONFIG_MODNET50		1	/* on an ModNET50 Board      */

#undef CONFIG_USE_IRQ			/* don't need them anymore */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_NETARMETH 1

/*
 * select serial console configuration
 */
#define CONFIG_NETARM_SERIAL
#define CONFIG_SERIAL1		1	/* we use Serial line 1 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		38400

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_JFFS2


#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.30.2
#define CONFIG_SERVERIP         192.168.30.122
#define CONFIG_SYS_ETH_PHY_ADDR        0x100
#define CONFIG_CMDLINE_TAG      /* submit bootargs to kernel */

/*#define CONFIG_BOOTDELAY	10*/
/* args and cmd for uClinux-image @ 0x10020000, ramdisk-image @ 0x100a0000 */
#define CONFIG_BOOTCOMMAND	"bootm 0x10020000 0x100a0000"
#define CONFIG_BOOTARGS		"console=ttyS0,38400 initrd=0x100a0040,530K " \
					"root=/dev/ram keepinitrd"

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory	*/
#define	CONFIG_SYS_PROMPT		"modnet50 # "	/* Monitor Command Prompt */
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size */
#define	CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00800000	/* 4 ... 8 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x00500000	/* default load address	*/

#define	CONFIG_SYS_HZ			900		/* decrementer freq: 2 kHz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
*/
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 banks of DRAM */
#define PHYS_SDRAM_1		0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x01000000 /* 16 MB */
#define PHYS_SDRAM_2		0x01000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_2_SIZE	0x01000000 /* 16 MB */

#define PHYS_FLASH_1		0x10000000 /* Flash Bank #1 */
#define PHYS_FLASH_1_SIZE	0x00200000 /* 2 MB (one chip only, 16bit access) */

#define PHYS_FLASH_2            0x10200001
#define PHYS_FLASH_2_SIZE       0x00200000

#define CONFIG_NETARM_EEPROM
/* #ifdef CONFIG_NETARM_EEPROM */
#define PHYS_NVRAM_1		0x20000000 /* EEPROM Bank #1 */
#define PHYS_NVRAM_SIZE		0x00002000 /* 8 KB */
/* #endif */

#define PHYS_EXT_1		0x30000000 /* Extensions Bank #1 */
#define PHYS_EXT_SIZE		0x01000000 /* 32 MB memory mapped I/O */

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_FLASH_SIZE		PHYS_FLASH_1_SIZE

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	35	/* max number of sectors on one chip */
#define CONFIG_SYS_MAIN_SECT_SIZE      0x00010000  /* main size of sectors on one chip */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* environment settings */
#define	CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_NOWHERE

#define CONFIG_ENV_ADDR		0x1001C000 /* environment start address */
#define CONFIG_ENV_SECT_SIZE       0x10000 /* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE		0x4000	/* max size for environment */

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00080000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=modnet50-0"
#define MTDPARTS_DEFAULT	"mtdparts=modnet50-0:-@512k(jffs2)"
*/

#endif	/* __CONFIG_H */
