/*
 * include/configs/mx1ads.h
 *
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
 *
 * This is the Configuration setting for Motorola MX1ADS board
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T		1	/* This is an ARM920T Core		*/
#define CONFIG_IMX		1	/* It's a Motorola MC9328 SoC		*/
#define CONFIG_MX1ADS		1	/* on a Motorola MX1ADS Board		*/
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff		*/

/*
 * Select serial console configuration
  */
#define CONFIG_IMX_SERIAL1		/* internal uart 1 */
/* #define _CONFIG_UART2 */		/* internal uart 2 */
/* #define CONFIG_SILENT_CONSOLE */	/* use this to disable output */

#define BOARD_LATE_INIT		1
#define USE_920T_MMU		1

#if 0
#define CFG_MX1_GPCR		0x000003AB	/* for MX1ADS 0L44N		*/
#define CFG_MX1_GPCR		0x000003AB	/* for MX1ADS 0L44N		*/
#define CFG_MX1_GPCR		0x000003AB	/* for MX1ADS 0L44N		*/
#endif

/*
 * Size of malloc() pool
 */

#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 *  CS8900 Ethernet drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		0x15000300
#define CS8900_BUS16		1	/* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */

/* #define CONFIG_UART1			*/
/* #define CONFIG_UART2		1	*/

#define CONFIG_BAUDRATE		115200


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_ELF


#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"root=/dev/msdk mem=48M"
#define CONFIG_BOOTFILE		"mx1ads"
#define CONFIG_BOOTCOMMAND	"tftp; bootm"

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
						/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */

#define CFG_HUSH_PARSER		1
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_LONGHELP				/* undef to save memory		*/

#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT		"MX1ADS$ "	/* Monitor Command Prompt */
#else
#define CFG_PROMPT		"MX1ADS=> "	/* Monitor Command Prompt */
#endif

#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
						/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x09000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0AF00000	/* 63 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ				/* everything, incl board info, in Hz */
#define CFG_LOAD_ADDR		0x08800000	/* default load address */
/*#define	CFG_HZ			1000 */
#define CFG_HZ			3686400
#define CFG_CPUSPEED		0x141

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

#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of SDRAM	*/
#define PHYS_SDRAM_1		0x08000000	/* SDRAM  on CSD0		*/
#define PHYS_SDRAM_1_SIZE	0x04000000	/* 64 MB			*/

#define CFG_MAX_FLASH_BANKS	1		/* 1 bank of SyncFlash		*/
#define CFG_FLASH_BASE		0x0C000000	/* SyncFlash on CSD1		*/
#define FLASH_BANK_SIZE		0x01000000	/* 16 MB Total			*/

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYNCFLASH	1
#define PHYS_FLASH_SIZE		0x01000000
#define CFG_MAX_FLASH_SECT	(16)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE+0x00ff8000)

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x04000 /* Total Size of Environment Sector */
#define CFG_ENV_SECT_SIZE	0x100000

/*-----------------------------------------------------------------------
 * Enable passing ATAGS
 */

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1

#define CONFIG_SYS_CLK_FREQ 16780000
#define CONFIG_SYSPLL_CLK_FREQ 16000000

#endif	/* __CONFIG_H */
