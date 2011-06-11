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
#define CONFIG_IMX_SERIAL
#define CONFIG_IMX_SERIAL1		/* internal uart 1 */
/* #define _CONFIG_UART2 */		/* internal uart 2 */
/* #define CONFIG_SILENT_CONSOLE */	/* use this to disable output */

#define BOARD_LATE_INIT		1
#define USE_920T_MMU		1

#if 0
#define CONFIG_SYS_MX1_GPCR		0x000003AB	/* for MX1ADS 0L44N		*/
#define CONFIG_SYS_MX1_GPCR		0x000003AB	/* for MX1ADS 0L44N		*/
#define CONFIG_SYS_MX1_GPCR		0x000003AB	/* for MX1ADS 0L44N		*/
#endif

/*
 * Size of malloc() pool
 */

#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/*
 *  CS8900 Ethernet drivers
 */
#define CONFIG_NET_MULTI
#define CONFIG_CS8900		/* we have a CS8900 on-board */
#define CONFIG_CS8900_BASE	0x15000300
#define CONFIG_CS8900_BUS16	/* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */

/* #define CONFIG_UART1			*/
/* #define CONFIG_UART2		1	*/

#define CONFIG_BAUDRATE		115200

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

#define CONFIG_SYS_HUSH_PARSER		1
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/

#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"MX1ADS$ "	/* Monitor Command Prompt */
#else
#define CONFIG_SYS_PROMPT		"MX1ADS=> "	/* Monitor Command Prompt */
#endif

#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
						/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x09000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0AF00000	/* 63 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x08800000	/* default load address */
/*#define	CONFIG_SYS_HZ			1000 */
#define CONFIG_SYS_HZ			3686400
#define CONFIG_SYS_CPUSPEED		0x141

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

#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of SDRAM	*/
#define PHYS_SDRAM_1		0x08000000	/* SDRAM  on CSD0		*/
#define PHYS_SDRAM_1_SIZE	0x04000000	/* 64 MB			*/

#define CONFIG_SYS_TEXT_BASE	0x10000000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x00300000
#define CONFIG_SYS_INIT_RAM_SIZE	0x000FFFFF
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_GBL_DATA_OFFSET)

#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* 1 bank of SyncFlash		*/
#define CONFIG_SYS_FLASH_BASE		0x0C000000	/* SyncFlash on CSD1		*/
#define FLASH_BANK_SIZE		0x01000000	/* 16 MB Total			*/

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYNCFLASH	1
#define PHYS_FLASH_SIZE		0x01000000
#define CONFIG_SYS_MAX_FLASH_SECT	(16)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE+0x00ff8000)

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x04000 /* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x100000

/*-----------------------------------------------------------------------
 * Enable passing ATAGS
 */

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1

#define CONFIG_SYS_CLK_FREQ 16780000
#define CONFIG_SYSPLL_CLK_FREQ 16000000

#endif	/* __CONFIG_H */
