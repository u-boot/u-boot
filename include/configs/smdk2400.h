/*
 * (C) Copyright 2002-2005
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 *
 * Configuation settings for the SAMSUNG board.
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
#define CONFIG_ARM920T		1	/* This is an ARM920T core	*/
#define CONFIG_S3C2400		1	/* in a SAMSUNG S3C2400 SoC	*/
#define CONFIG_SMDK2400		1	/* on an SAMSUNG SMDK2400 Board */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000 /* SMDK2400 has 12 MHz input clock */
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1


/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		0x07000300 /* agrees with WIN CE PA */
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SAMSUNG */

#undef	CONFIG_HWFLOW			/* include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* enable modem initialization stuff */

/*
 * The following enables modem debugging stuff. The dbg() and
 * 'char screen[1024]' are used for debug printfs. Unfortunately,
 * it is usable only from BDI
 */
#undef CONFIG_MODEM_SUPPORT_DEBUG

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

#define	CONFIG_TIMESTAMP	1	/* Print timestamp info for images */

/* Use s3c2400's RTC */
#define CONFIG_RTC_S3C24X0	1

#ifndef USE_920T_MMU
#define CONFIG_COMMANDS_tmp    ((CONFIG_CMD_DFL & ~CFG_CMD_CACHE) | \
				CFG_CMD_DATE	| \
				CFG_CMD_SNTP	)
#else
#define CONFIG_COMMANDS_tmp    (CONFIG_CMD_DFL	| \
				CFG_CMD_DATE	| \
				CFG_CMD_SNTP	)
#endif

#ifdef CONFIG_HWFLOW
#define CONFIG_COMMANDS		(CONFIG_COMMANDS_tmp | CFG_CMD_HWFLOW)
#else
#define CONFIG_COMMANDS		CONFIG_COMMANDS_tmp
#endif

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#if 0
#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600"
#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#endif
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		134.98.93.36
#define CONFIG_SERVERIP		134.98.93.22
#if 0
#define CONFIG_BOOTFILE		"elinos-lart"
#define CONFIG_BOOTCOMMAND	"tftp; bootm"
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"SMDK2400 # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0c000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0e000000	/* 32 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x0cf00000	/* default load address	*/

/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define	CFG_HZ			1562500

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

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
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x0c000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */

#define CFG_FLASH_BASE		0x00000000 /* Flash Bank #1 */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(64)	/* max number of sectors on one chip */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x40000)
#define CFG_ENV_SIZE		0x40000

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

#endif	/* __CONFIG_H */
