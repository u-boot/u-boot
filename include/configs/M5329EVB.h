/*
 * Configuation settings for the Freescale MCF5329 FireEngine board.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef _M5329EVB_H
#define _M5329EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF532x		/* define processor family */
#define CONFIG_M5329		/* define processor type */

#undef DEBUG

#define CONFIG_MCFSERIAL
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

#undef CONFIG_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT	5000	/* timeout in milliseconds, max timeout is 6.71sec */

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL | \
							  CFG_CMD_CACHE | \
							  CFG_CMD_DATE | \
							  CFG_CMD_ELF | \
							  CFG_CMD_FLASH | \
							  (CFG_CMD_LOADB | CFG_CMD_LOADS) | \
							  CFG_CMD_MEMORY | \
							  CFG_CMD_MISC | \
							  CFG_CMD_MII | \
							  CFG_CMD_NET | \
							  CFG_CMD_PING | \
							  CFG_CMD_REGINFO \
							)

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_MULTI	1
#	define CONFIG_MII		1
#	define CFG_DISCOVER_PHY
#	define CFG_RX_ETH_BUFFER	8
#	define CFG_FAULT_ECHO_LINK_DOWN

#	define CFG_FEC0_PINMUX	0
#	define CFG_FEC0_MIIBASE	CFG_FEC0_IOBASE
#	define MCFFEC_TOUT_LOOP 50000
/* If CFG_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CFG_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CFG_FAULT_ECHO_LINK_DOWN
#			define CFG_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CFG_DISCOVER_PHY */
#endif

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)

#define CONFIG_MCFRTC
#undef RTC_DEBUG

/* Timer */
#define CONFIG_MCFTMR
#undef CONFIG_MCFPIT

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>
#define CONFIG_BOOTDELAY	1	/* autoboot after 5 seconds */
#ifdef CONFIG_MCFFEC
#	define CONFIG_ETHADDR		00:e0:0c:bc:e5:60
#	define CONFIG_IPADDR		192.162.1.2
#	define CONFIG_NETMASK		255.255.255.0
#	define CONFIG_SERVERIP		192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#	define CONFIG_OVERWRITE_ETHADDR_ONCE
#endif				/* FEC_ENET */

#define CONFIG_HOSTNAME		M5329EVB
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"			\
	"loadaddr=40010000\0"	\
	"u-boot=u-boot.bin\0"	\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"	\
	"prog=prot off 0 2ffff;"	\
	"era 0 2ffff;"	\
	"cp.b ${loadaddr} 0 ${filesize};"	\
	"save\0"	\
	""

#define CONFIG_PRAM			512	/* 512 KB */
#define CFG_PROMPT			"-> "
#define CFG_LONGHELP		/* undef to save memory */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#	define CFG_CBSIZE			1024	/* Console I/O Buffer Size */
#else
#	define CFG_CBSIZE			256	/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE			(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS			16	/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size    */
#define CFG_LOAD_ADDR		0x40010000

#define CFG_HZ				1000
#define CFG_CLK				80000000
#define CFG_CPU_CLK			CFG_CLK * 3

#define CFG_MBAR			0xFC000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x80000000
#define CFG_INIT_RAM_END	0x8000	/* End of used area in internal SRAM */
#define CFG_INIT_RAM_CTRL	0x221
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x40000000
#define CFG_SDRAM_SIZE		16	/* SDRAM size in MB */
#define CFG_SDRAM_CFG1		0x53722730
#define CFG_SDRAM_CFG2		0x56670000
#define CFG_SDRAM_CTRL		0xE1092000
#define CFG_SDRAM_EMOD		0x40010000
#define CFG_SDRAM_MODE		0x018D0000

#define CFG_MEMTEST_START	CFG_SDRAM_BASE + 0x400
#define CFG_MEMTEST_END		((CFG_SDRAM_SIZE - 3) << 20)

#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

#define CFG_BOOTPARAMS_LEN	64*1024
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_CFI
#ifdef CFG_FLASH_CFI
#	define CFG_FLASH_CFI_DRIVER	1
#	define CFG_FLASH_SIZE		0x800000	/* Max size that the board might have */
#	define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CFG_MAX_FLASH_BANKS		1	/* max number of memory banks */
#	define CFG_MAX_FLASH_SECT		137	/* max number of sectors on one chip */
#	define CFG_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */
#endif

#define CFG_FLASH_BASE			0
#define CFG_FLASH0_BASE			(CFG_CS0_BASE << 16)

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#define CFG_ENV_OFFSET		0x4000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_IS_EMBEDDED	1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Chipselect bank definitions
 */
/*
 * CS0 - NOR Flash 1, 2, 4, or 8MB
 * CS1 - CompactFlash and registers
 * CS2 - NAND Flash 16, 32, or 64MB
 * CS3 - Available
 * CS4 - Available
 * CS5 - Available
 */
#define CFG_CS0_BASE		0
#define CFG_CS0_MASK		0x007f0001
#define CFG_CS0_CTRL		0x00001fa0

#define CFG_CS1_BASE		0x1000
#define CFG_CS1_MASK		0x001f0001
#define CFG_CS1_CTRL		0x002A3780

#ifdef NANDFLASH_SIZE
#define CFG_CS2_BASE		0x00800000
#define CFG_CS2_MASK		0x00ff0001
#define CFG_CS2_CTRL		0x00001f60
#endif

#define CONFIG_UDP_CHECKSUM

#endif				/* _M5329EVB_H */
