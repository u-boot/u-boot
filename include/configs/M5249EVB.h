/*
 * Configuation settings for the esd TASREG board.
 *
 * (C) Copyright 2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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

#ifndef _M5249EVB_H
#define _M5249EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF52x2			/* define processor family */
#define CONFIG_M5249			/* define processor type */

#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)
#define CONFIG_BAUDRATE		19200
#define CFG_BAUDRATE_TABLE { 9600 , 19200 , 38400 , 57600, 115200 }

#undef  CONFIG_WATCHDOG

#undef CONFIG_MONITOR_IS_IN_RAM		/* no pre-loader required!!! ;-) */

/*
 * BOOTP options
 */
#undef CONFIG_BOOTP_BOOTFILESIZE
#undef CONFIG_BOOTP_BOOTPATH
#undef CONFIG_BOOTP_GATEWAY
#undef CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_NET

#define CFG_PROMPT		"=> "
#define CFG_LONGHELP				/* undef to save memory		*/

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_DEVICE_NULLDEV	1	/* include nulldev device	*/
#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup	*/
#define CONFIG_AUTO_COMPLETE	1	/* add autocompletion support	*/
#define CONFIG_LOOPW		1	/* enable loopw command	*/
#define CONFIG_MX_CYCLIC	1	/* enable mdc/mwc commands	*/

#define CFG_LOAD_ADDR		0x200000	/* default load address */

#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

#define CFG_HZ			1000

/*
 * Clock configuration: enable only one of the following options
 */

#undef  CFG_PLL_BYPASS				/* bypass PLL for test purpose */
#define CFG_FAST_CLK		1		/* MCF5249 can run at 140MHz   */
#define	CFG_CLK			132025600	/* MCF5249 can run at 140MHz   */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_MBAR		0x10000000	/* Register Base Addrs */
#define	CFG_MBAR2		0x80000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x20000000
#define CFG_INIT_RAM_END	0x1000	/* End of used area in internal SRAM	*/
#define CFG_GBL_DATA_SIZE	64	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x4000	/* Address of Environment Sector*/
#define CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x2000 /* see README - env sector total size	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_SIZE		16		/* SDRAM size in MB */
#define CFG_FLASH_BASE		(CFG_CSAR0 << 16)

#if 0 /* test-only */
#define CONFIG_PRAM		512 /* test-only for SDRAM problem!!!!!!!!!!!!!!!!!!!! */
#endif

#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)

#define CFG_MONITOR_LEN		0x20000
#define CFG_MALLOC_LEN		(1 * 1024*1024)	/* Reserve 1 MB for malloc()	*/
#define CFG_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_BOOTMAPSZ		(CFG_SDRAM_BASE + (CFG_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_CFI
#ifdef CFG_FLASH_CFI

#	define CFG_FLASH_CFI_DRIVER	1
#	define CFG_FLASH_SIZE		0x1000000	/* Max size that the board might have */
#	define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#	define CFG_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#	define CFG_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */
#	define CFG_FLASH_CHECKSUM
#	define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */

/* CS0 - AMD Flash, address 0xffc00000 */
#define	CFG_CSAR0		0xffe0
#define	CFG_CSCR0		0x1980		/* WS=0110, AA=1, PS=10         */
/** Note: There is a CSMR0/DRAM vector problem, need to disable C/I ***/
#define	CFG_CSMR0		0x003f0021	/* 4MB, AA=0, WP=0, C/I=1, V=1  */

/* CS1 - FPGA, address 0xe0000000 */
#define	CFG_CSAR1		0xe000
#define	CFG_CSCR1		0x0d80		/* WS=0011, AA=1, PS=10         */
#define	CFG_CSMR1		0x00010001	/* 128kB, AA=0, WP=0, C/I=0, V=1*/

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define	CFG_GPIO_FUNC		0x00000008	/* Set gpio pins: none          */
#define	CFG_GPIO1_FUNC		0x00df00f0	/* 36-39(SWITCH),48-52(FPGAs),54*/
#define	CFG_GPIO_EN		0x00000008	/* Set gpio output enable       */
#define	CFG_GPIO1_EN		0x00c70000	/* Set gpio output enable       */
#define	CFG_GPIO_OUT		0x00000008	/* Set outputs to default state */
#define	CFG_GPIO1_OUT		0x00c70000	/* Set outputs to default state */
#define CFG_GPIO1_LED		0x00400000	/* user led                     */

#endif	/* M5249 */

