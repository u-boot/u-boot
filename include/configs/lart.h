/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the LART board.
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
#define CONFIG_SA1100		1	/* This is an SA1100 CPU	*/
#define CONFIG_LART		1	/* on an LART Board      */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */
/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_NO_DCACHE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		0x20008300
#define CS8900_BUS16		1

/*
 * select serial console configuration
 */
#define CONFIG_SA1100_SERIAL
#define CONFIG_SERIAL3          1	/* we use SERIAL 3 on LART */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		9600


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>


#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"root=ramfs devfs=mount console=ttySA0,9600"
#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK          255.255.0.0
#define CONFIG_IPADDR		172.22.2.131
#define CONFIG_SERVERIP		172.22.2.126
#define CONFIG_BOOTFILE		"elinos-lart"
#define CONFIG_BOOTCOMMAND	"tftp; bootm"

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"LART # "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0xc0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0xc0800000	/* 4 ... 8 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0xc8000000	/* default load address	*/

#define	CONFIG_SYS_HZ			3686400		/* incrementer freq: 3.6864 MHz */
#define CONFIG_SYS_CPUSPEED		0x0b		/* set core clock to 220 MHz */

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
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1		0xc0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x00800000 /* 8 MB */
#define PHYS_SDRAM_2		0xc1000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00800000 /* 8 MB */
#define PHYS_SDRAM_3            0xc8000000 /* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE       0x00800000 /* 8 MB */
#define PHYS_SDRAM_4            0xc9000000 /* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE       0x00800000 /* 8 MB */


#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x00400000 /* 4 MB */

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	(31+8)	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + 0x1C000)	/* Addr of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

#endif	/* __CONFIG_H */
