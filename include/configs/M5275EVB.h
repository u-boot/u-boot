/*
 * Configuation settings for the Motorola MC5275EVB board.
 *
 * By Arthur Shipkowski <art@videon-central.com>
 * Copyright (C) 2005 Videon Central, Inc.
 *
 * Based off of M5272C3 board code by Josef Baumgartner
 * <josef.baumgartner@telex.de>
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

#ifndef _M5275EVB_H
#define _M5275EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF52x2			/* define processor family */
#define CONFIG_M5275			/* define processor type */
#define CONFIG_M5275EVB			/* define board type */

#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)
#define CONFIG_BAUDRATE		19200
#define CFG_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

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
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* Available command configuration */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_DHCP

#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_LOADB

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#define CONFIG_NET_MULTI	1
#define CONFIG_MII		1
#define CONFIG_MII_INIT		1
#define CFG_DISCOVER_PHY
#define CFG_RX_ETH_BUFFER	8
#define CFG_FAULT_ECHO_LINK_DOWN
#define CFG_FEC0_PINMUX		0
#define CFG_FEC0_MIIBASE	CFG_FEC0_IOBASE
#define CFG_FEC1_PINMUX		0
#define CFG_FEC1_MIIBASE	CFG_FEC1_IOBASE
#define MCFFEC_TOUT_LOOP	50000
#define CONFIG_HAS_ETH1
/* If CFG_DISCOVER_PHY is not defined - hardcoded */
#ifndef CFG_DISCOVER_PHY
#define FECDUPLEX		FULL
#define FECSPEED		_100BASET
#else
#ifndef CFG_FAULT_ECHO_LINK_DOWN
#define CFG_FAULT_ECHO_LINK_DOWN
#endif
#endif
#endif

/* I2C */
#define CONFIG_FSL_I2C
#define CONFIG_HARD_I2C		/* I2C with hw support */
#undef CONFIG_SOFT_I2C
#define CFG_I2C_SPEED		80000
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_OFFSET		0x00000300
#define CFG_IMMR		CFG_MBAR

#ifdef CONFIG_MCFFEC
#define CONFIG_ETHADDR		00:06:3b:01:41:55
#define CONFIG_ETH1ADDR		00:0e:0c:bc:e5:60
#endif

#define CFG_PROMPT		"-> "
#define CFG_LONGHELP		/* undef to save memory	*/

#if (CONFIG_CMD_KGDB)
#	define CFG_CBSIZE	1024
#else
#	define CFG_CBSIZE	256
#endif
#define CFG_PBSIZE		(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS		16
#define CFG_BARGSIZE		CFG_CBSIZE

#define CFG_LOAD_ADDR		0x800000

#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOTCOMMAND	"bootm ffe40000"
#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

#define CFG_HZ			1000
#define CFG_CLK			150000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_MBAR		0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x20000000
#define CFG_INIT_RAM_END	0x10000	/* End of used area in internal SRAM */
#define CFG_GBL_DATA_SIZE	1000	/* bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_SIZE		16	/* SDRAM size in MB */
#define CFG_FLASH_BASE		0xffe00000

#ifdef CONFIG_MONITOR_IS_IN_RAM
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
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial mmap for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	11	/* max number of sectors on one chip */
#define CFG_FLASH_ERASE_TOUT	1000

#define CFG_FLASH_CFI		1
#define CFG_FLASH_CFI_DRIVER	1
#define CFG_FLASH_SIZE		0x200000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */
#define CFG_AR0_PRELIM		(CFG_FLASH_BASE >> 16)
#define CFG_CR0_PRELIM		0x1980
#define CFG_MR0_PRELIM		0x001F0001

#define CFG_AR1_PRELIM		0x3000
#define CFG_CR1_PRELIM		0x1900
#define CFG_MR1_PRELIM		0x00070001

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_FECI2C		0x0FA0

#endif	/* _M5275EVB_H */
