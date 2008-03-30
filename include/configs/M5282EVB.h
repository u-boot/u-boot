/*
 * Configuation settings for the Motorola MC5282EVB board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#ifndef _CONFIG_M5282EVB_H
#define _CONFIG_M5282EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define	CONFIG_MCF52x2		/* define processor family */
#define CONFIG_M5282		/* define processor type */

#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)
#define CONFIG_BAUDRATE 19200
#define CFG_BAUDRATE_TABLE { 9600 , 19200 , 38400 , 57600, 115200 }

#undef	CONFIG_MONITOR_IS_IN_RAM	/* define if monitor is started from a pre-loader */

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#define CFG_ENV_ADDR		0xffe04000
#define CFG_ENV_SIZE		0x2000
#define CFG_ENV_IS_IN_FLASH	1

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
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_MII

#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_LOADB

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_MULTI		1
#	define CONFIG_MII		1
#	define CONFIG_MII_INIT		1
#	define CFG_DISCOVER_PHY
#	define CFG_RX_ETH_BUFFER	8
#	define CFG_FAULT_ECHO_LINK_DOWN

#	define CFG_FEC0_PINMUX		0
#	define CFG_FEC0_MIIBASE		CFG_FEC0_IOBASE
#	define MCFFEC_TOUT_LOOP 	50000
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

#define CONFIG_BOOTDELAY	5
#ifdef CONFIG_MCFFEC
#	define CONFIG_ETHADDR	00:e0:0c:bc:e5:60
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#	define CONFIG_OVERWRITE_ETHADDR_ONCE
#endif				/* CONFIG_MCFFEC */

#define CONFIG_HOSTNAME		M5272C3
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"loadaddr=10000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off ffe00000 ffe3ffff;"	\
	"era ffe00000 ffe3ffff;"		\
	"cp.b ${loadaddr} ffe00000 ${filesize};"\
	"save\0"				\
	""

#define CFG_PROMPT		"-> "
#define	CFG_LONGHELP		/* undef to save memory         */

#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE		1024	/* Console I/O Buffer Size      */
#else
#define	CFG_CBSIZE		256	/* Console I/O Buffer Size      */
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define	CFG_MAXARGS		16	/* max number of command args   */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size    */

#define CFG_LOAD_ADDR		0x20000

#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

#define CFG_HZ			1000000
#define	CFG_CLK			64000000

/* PLL Configuration: Ext Clock * 6 (see table 9-4 of MCF user manual) */

#define CFG_MFD			0x02	/* PLL Multiplication Factor Devider */
#define CFG_RFD			0x00	/* PLL Reduce Frecuency Devider */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define	CFG_MBAR		0x40000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x20000000
#define CFG_INIT_RAM_END	0x10000	/* End of used area in internal SRAM    */
#define CFG_GBL_DATA_SIZE	64	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define	CFG_SDRAM_SIZE		16	/* SDRAM size in MB */
#define CFG_FLASH_BASE		0xffe00000
#define	CFG_INT_FLASH_BASE	0xf0000000
#define CFG_INT_FLASH_ENABLE	0x21

/* If M5282 port is fully implemented the monitor base will be behind
 * the vector table. */
#if (TEXT_BASE != CFG_INT_FLASH_BASE)
#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)
#else
#define CFG_MONITOR_BASE	(TEXT_BASE + 0x418)	/* 24 Byte for CFM-Config */
#endif

#define CFG_MONITOR_LEN		0x20000
#define CFG_MALLOC_LEN		(256 << 10)
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
#define CFG_CS0_BASE		CFG_FLASH_BASE
#define CFG_CS0_SIZE		2*1024*1024
#define CFG_CS0_WIDTH		16
#define CFG_CS0_RO 		0
#define CFG_CS0_WS		6
/*
#define CFG_CS3_BASE		0xE0000000
#define CFG_CS3_SIZE		1*1024*1024
#define CFG_CS3_WIDTH		16
#define CFG_CS3_RO 		0
#define CFG_CS3_WS		6
*/
/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_PACNT		0x0000000	/* Port A D[31:24] */
#define CFG_PADDR		0x0000000
#define CFG_PADAT		0x0000000

#define CFG_PBCNT		0x0000000	/* Port B D[23:16] */
#define CFG_PBDDR		0x0000000
#define CFG_PBDAT		0x0000000

#define CFG_PCCNT		0x0000000	/* Port C D[15:08] */
#define CFG_PCDDR		0x0000000
#define CFG_PCDAT		0x0000000

#define CFG_PDCNT		0x0000000	/* Port D D[07:00] */
#define CFG_PCDDR		0x0000000
#define CFG_PCDAT		0x0000000

#define CFG_PEHLPAR		0xC0
#define CFG_PUAPAR		0x0F	/* UA0..UA3 = Uart 0 +1 */
#define CFG_DDRUA		0x05
#define CFG_PJPAR 		0xFF;

#endif				/* _CONFIG_M5282EVB_H */
