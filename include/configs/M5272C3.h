/*
 * Configuation settings for the Motorola MC5272C3 board.
 *
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M5272C3_H
#define _M5272C3_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF52x2			/* define processor family */
#define CONFIG_M5272			/* define processor type */

#define FEC_ENET

#define CONFIG_BAUDRATE		19200
#define CFG_BAUDRATE_TABLE { 9600 , 19200 , 38400 , 57600, 115200 }

#define CONFIG_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT 10000	/* timeout in milliseconds */

#define CONFIG_MONITOR_IS_IN_RAM	/* define if monitor is started from a pre-loader */

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#ifndef CONFIG_MONITOR_IS_IN_RAM
#define CFG_ENV_OFFSET		0x4000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_IS_EMBEDDED	1
#else
#define CFG_ENV_ADDR		0xffe04000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#endif


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_MII

#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_LOADB


#define CONFIG_BOOTDELAY	5

#define CFG_PROMPT		"-> "
#define CFG_LONGHELP				/* undef to save memory		*/

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_LOAD_ADDR		0x20000

#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

#define CFG_HZ			1000
#define CFG_CLK			66000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_MBAR		0x10000000	/* Register Base Addrs */

#define CFG_SCR			0x0003;
#define CFG_SPR			0xffff;

#define CFG_DISCOVER_PHY
#define CFG_ENET_BD_BASE	0x380000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x20000000
#define CFG_INIT_RAM_END	0x1000	/* End of used area in internal SRAM	*/
#define CFG_GBL_DATA_SIZE	64	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_SIZE		4		/* SDRAM size in MB */
#define CFG_FLASH_BASE		0xffe00000

#ifdef	CONFIG_MONITOR_IS_IN_RAM
#define CFG_MONITOR_BASE	0x20000
#else
#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)
#endif

#define CFG_MONITOR_LEN		0x20000
#define CFG_MALLOC_LEN		(256 << 10)
#define CFG_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	11	/* max number of sectors on one chip	*/
#define CFG_FLASH_ERASE_TOUT	1000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */
#define CFG_BR0_PRELIM		0xFFE00201
#define CFG_OR0_PRELIM		0xFFE00014

#define CFG_BR1_PRELIM		0
#define CFG_OR1_PRELIM		0

#define CFG_BR2_PRELIM		0x30000001
#define CFG_OR2_PRELIM		0xFFF80000

#define CFG_BR3_PRELIM		0
#define CFG_OR3_PRELIM		0

#define CFG_BR4_PRELIM		0
#define CFG_OR4_PRELIM		0

#define CFG_BR5_PRELIM		0
#define CFG_OR5_PRELIM		0

#define CFG_BR6_PRELIM		0
#define CFG_OR6_PRELIM		0

#define CFG_BR7_PRELIM		0x00000701
#define CFG_OR7_PRELIM		0xFFC0007C

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_PACNT		0x00000000
#define CFG_PADDR		0x0000
#define CFG_PADAT		0x0000
#define CFG_PBCNT		0x55554155		/* Ethernet/UART configuration */
#define CFG_PBDDR		0x0000
#define CFG_PBDAT		0x0000
#define CFG_PDCNT		0x00000000

#endif	/* _M5272C3_H */
