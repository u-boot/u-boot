/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
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
 * If we are developing, we might want to start armboot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#define CONFIG_INIT_CRITICAL		/* undef for developing */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define	CONFIG_S3C2410		1	/* in a SAMSUNG S3C2410 SoC     */
#define CONFIG_VCMA9		1	/* on a MPL VCMA9 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000/* VCMA9 has 12MHz input clock	*/

#define USE_920T_MMU		1
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs    	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

/***********************************************************
 * Command definition
 ***********************************************************/
#define CONFIG_COMMANDS \
			(CONFIG_CMD_DFL	 | \
			CFG_CMD_CACHE	 | \
			CFG_CMD_EEPROM	 | \
			CFG_CMD_I2C	 | \
			CFG_CMD_REGINFO  | \
			CFG_CMD_ELF	 | \
			CFG_CMD_BSP)

/* this must be included after the definiton of CONFIG_COMMANDS */
#include <cmd_confdefs.h>

#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
/***********************************************************
 * I2C stuff:
 * the MPL VCMA9 is equipped with an ATMEL 24C256 EEPROM at
 * address 0x50 with 16bit addressing
 ***********************************************************/
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#define CFG_I2C_SPEED 		100000	/* I2C speed */
#define CFG_I2C_SLAVE 		0x7F	/* I2C slave addr */

#define CFG_I2C_EEPROM_ADDR	0x50
#define CFG_I2C_EEPROM_ADDR_LEN	2
#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x000	/* environment starts at offset 0 */
#define CFG_ENV_SIZE		0x800	/* 2KB should be more than enough */

#undef CFG_I2C_EEPROM_ADDR_OVERFLOW
#define CFG_EEPROM_PAGE_WRITE_BITS 6	/* 64 bytes page write mode on 24C256 */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

/*
 * Size of malloc() pool
 */
#define CONFIG_MALLOC_SIZE	(CFG_ENV_SIZE + 128*1024)

#define CFG_MONITOR_LEN		(256 * 1024)
#define CFG_MALLOC_LEN		(128 * 1024)

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1		/* we have a CS8900 on-board */
#define CS8900_BASE		0x20000300
#define CS8900_BUS16		1 		/* the Linux driver does accesses as shorts */

#define CONFIG_DRIVER_S3C24X0_I2C	1	/* we use the buildin I2C controller */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on VCMA9 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		9600

#define CONFIG_BOOTDELAY	3
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		10.0.0.110
#define CONFIG_SERVERIP		10.0.0.1

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"VCMA9 # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x30000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x33F00000	/* 63 MB in DRAM	*/
#define CFG_ALT_MEMTEST
#define	CFG_LOAD_ADDR		0x33000000	/* default load address	*/


#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

/* we configure PWM Timer 4 to 1us ~ 1MHz */
/*#define	CFG_HZ			1000000 */
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
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_AMD_LV400	1	/* uncomment this if you have a LV400 flash */
#if 0
#define CONFIG_AMD_LV800	1	/* uncomment this if you have a LV800 flash */
#endif

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#ifdef CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x00100000 /* 1MB */
#define CFG_MAX_FLASH_SECT	(19)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x0F0000) /* addr of environment */
#endif
#ifdef CONFIG_AMD_LV400
#define PHYS_FLASH_SIZE		0x00080000 /* 512KB */
#define CFG_MAX_FLASH_SECT	(11)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x070000) /* addr of environment */
#endif

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

#if 0
#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector */
#endif

#define MULTI_PURPOSE_SOCKET_ADDR 0

#endif	/* __CONFIG_H */
