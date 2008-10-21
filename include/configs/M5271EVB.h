/*
 * Configuation settings for the Freescale M5271EVB
 *
 * Based on MC5272C3 and r5200 board configs
 * (C) Copyright 2006 Lab X Technologies <zachary.landau@labxtechnologies.com>
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

#ifndef _M5271EVB_H
#define _M5271EVB_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_MCF52x2		/* define processor family */
#define CONFIG_M5271		/* define processor type */
#define CONFIG_M5271EVB		/* define board type */

#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

#undef CONFIG_WATCHDOG		/* disable watchdog */

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#ifndef CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_ENV_OFFSET		0x4000
#else
#define CONFIG_ENV_ADDR		0xffe04000
#endif
#define CONFIG_ENV_SECT_SIZE	0x2000
#define CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

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

#define CONFIG_CMD_PING
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC

#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_LOADB

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_MULTI		1
#	define CONFIG_MII		1
#	define CONFIG_MII_INIT		1
#	define CONFIG_SYS_DISCOVER_PHY
#	define CONFIG_SYS_RX_ETH_BUFFER	8
#	define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

#	define CONFIG_SYS_FEC0_PINMUX		0
#	define CONFIG_SYS_FEC0_MIIBASE		CONFIG_SYS_FEC0_IOBASE
#	define MCFFEC_TOUT_LOOP		50000
/* If CONFIG_SYS_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CONFIG_SYS_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#			define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CONFIG_SYS_DISCOVER_PHY */
#endif

/* I2C */
#define CONFIG_FSL_I2C
#define CONFIG_HARD_I2C		/* I2C with hw support */
#undef CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SPEED		80000
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x00000300
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR

#define CONFIG_BOOTDELAY	1	/* autoboot after 5 seconds */
#define CONFIG_BOOTFILE		"u-boot.bin"
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_RETRY_COUNT	5
#	define CONFIG_ETHADDR	00:e0:0c:bc:e5:60
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#	define CONFIG_OVERWRITE_ETHADDR_ONCE
#endif				/* FEC_ENET */

#define CONFIG_HOSTNAME		M5235EVB
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"loadaddr=10000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off ffe00000 ffe2ffff;"		\
	"era ffe00000 ffe2ffff;"				\
	"cp.b ${loadaddr} 0 ${filesize};"	\
	"save\0"				\
	""

#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_LONGHELP		/* undef to save memory */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size      */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size      */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define CONFIG_SYS_MEMTEST_START	0x400
#define CONFIG_SYS_MEMTEST_END		0x380000

#define CONFIG_SYS_HZ			1000000
#define CONFIG_SYS_CLK			100000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_MBAR		0x40000000	/* Register Base Addrs */

/*
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_END	0x1000	/* End of used area in internal SRAM    */
#define CONFIG_SYS_GBL_DATA_SIZE	64	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		16	/* SDRAM size in MB */
#define CONFIG_SYS_FLASH_BASE		0xffe00000

#ifdef	CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_SYS_MONITOR_BASE	0x20000
#else
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#endif

#define CONFIG_SYS_MONITOR_LEN		0x40000
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
#define CONFIG_SYS_BOOTPARAMS_LEN	(64*1024)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))

/* FLASH organization */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks           */
#define CONFIG_SYS_MAX_FLASH_SECT	11	/* max number of sectors on one chip    */
#define CONFIG_SYS_FLASH_ERASE_TOUT	1000

#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER	1
#define CONFIG_SYS_FLASH_SIZE		0x200000

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE	16

/* Port configuration */
#define CONFIG_SYS_FECI2C		0xF0

#endif				/* _M5271EVB_H */
