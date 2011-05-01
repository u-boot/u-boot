/*
 * (C) Copyright 2002, 2003
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <garyj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the MPL VCMA9 board.
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
#define CONFIG_ARM920T	1	/* This is an ARM920T Core	*/
#define CONFIG_S3C24X0	1	/* in a SAMSUNG S3C24x0-type SoC	*/
#define CONFIG_S3C2410	1	/* specifically a SAMSUNG S3C2410 SoC	*/
#define CONFIG_VCMA9	1	/* on a MPL VCMA9 Board  */

#define CONFIG_SYS_TEXT_BASE	0x0

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000/* VCMA9 has 12MHz input clock	*/

#define USE_920T_MMU		1
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1


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

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_USB
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_FAT
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_BSP


#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
/***********************************************************
 * I2C stuff:
 * the MPL VCMA9 is equipped with an ATMEL 24C256 EEPROM at
 * address 0x50 with 16bit addressing
 ***********************************************************/
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed */
#define CONFIG_SYS_I2C_SLAVE		0x7F	/* I2C slave addr */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x000	/* environment starts at offset 0 */
#define CONFIG_ENV_SIZE		0x800	/* 2KB should be more than enough */

#undef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 6	/* 64 bytes page write mode on 24C256 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/*
 * Size of malloc() pool
 */
/*#define CONFIG_MALLOC_SIZE	(CONFIG_ENV_SIZE + 128*1024)*/

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* BUNZIP2 needs a lot of RAM */

/*
 * Hardware drivers
 */
#define CONFIG_NET_MULTI
#define CONFIG_CS8900		/* we have a CS8900 on-board */
#define CONFIG_CS8900_BASE	0x20000300
#define CONFIG_CS8900_BUS16	/* the Linux driver does accesses as shorts */

#define CONFIG_DRIVER_S3C24X0_I2C	1	/* we use the buildin I2C controller */

/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on VCMA9 */

/************************************************************
 * USB support
 ************************************************************/
#define CONFIG_USB_OHCI		1
#define CONFIG_USB_KEYBOARD	1
#define CONFIG_USB_STORAGE	1
#define CONFIG_DOS_PARTITION	1

/* Enable needed helper functions */
#define CONFIG_SYS_STDIO_DEREGISTER		/* needs stdio_deregister */

/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_S3C24X0	1


/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		9600

#define CONFIG_BOOTDELAY	5
/* autoboot (do NOT change this set environment variable "bootdelay" to -1 instead) */
/* #define CONFIG_BOOT_RETRY_TIME	-10	/XXX* feature is available but not enabled */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check console even if bootdelay = 0 */

#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		10.0.0.110
#define CONFIG_SERVERIP		10.0.0.1

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"VCMA9 # "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x30000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x30F80000	/* 15.5 MB in DRAM	*/

#define CONFIG_SYS_ALT_MEMTEST
#define	CONFIG_SYS_LOAD_ADDR		0x30800000	/* default load address	*/

/* we configure PWM Timer 4 to 1us ~ 1MHz */
/*#define	CONFIG_SYS_HZ			1000000 */
#define	CONFIG_SYS_HZ			1562500

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* support BZIP2 compression */
#define CONFIG_BZIP2		1

/************************************************************
 * Ident
 ************************************************************/
/*#define VERSION_TAG "released"*/
#define VERSION_TAG "unstable"
#define CONFIG_IDENT_STRING "\n(c) 2003 by MPL AG Switzerland, MEV-10080-001 " VERSION_TAG

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
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_AMD_LV400	1	/* uncomment this if you have a LV400 flash */
#if 0
#define CONFIG_AMD_LV800	1	/* uncomment this if you have a LV800 flash */
#endif

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#ifdef CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x00100000 /* 1MB */
#define CONFIG_SYS_MAX_FLASH_SECT	(19)	/* max number of sectors on one chip */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x0F0000) /* addr of environment */
#endif
#ifdef CONFIG_AMD_LV400
#define PHYS_FLASH_SIZE		0x00080000 /* 512KB */
#define CONFIG_SYS_MAX_FLASH_SECT	(11)	/* max number of sectors on one chip */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x070000) /* addr of environment */
#endif

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(5*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(5*CONFIG_SYS_HZ) /* Timeout for Flash Write */

#if 0
#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector */
#endif


#define CONFIG_SYS_JFFS2_FIRST_BANK	0
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#define MULTI_PURPOSE_SOCKET_ADDR 0x08000000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
					GENERATED_GBL_DATA_SIZE)


#endif	/* __CONFIG_H */
