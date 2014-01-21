/*
 * Configuation settings for the Freescale M5271EVB
 *
 * Based on MC5272C3 and r5200 board configs
 * (C) Copyright 2006 Lab X Technologies <zachary.landau@labxtechnologies.com>
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_PING
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC

#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_LOADB
#define CONFIG_CMDLINE_EDITING	1 /* enables command line history */
#define CONFIG_SYS_HUSH_PARSER /* Use the HUSH parser */

#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
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
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	80000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x00000300
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR

#define CONFIG_BOOTDELAY	1	/* autoboot after 1 seconds */
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

#define CONFIG_HOSTNAME		M5271EVB
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"loadaddr=10000\0"			\
	"uboot=u-boot.bin\0"		\
	"load=tftp $loadaddr $uboot\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off ffe00000 ffe3ffff;"	\
	"era ffe00000 ffe3ffff;"		\
	"cp.b $loadaddr ffe00000 $filesize;"	\
	"save\0"				\
	""

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

/* Clock configuration
 * The external oscillator is a 25.000 MHz
 * CONFIG_SYS_CLK for ColdFire V2 sets cpu_clk (not bus_clk)
 * bus_clk = (cpu_clk/2) (fixed ratio)
 *
 * If CONFIG_SYS_CLK is changed. the CONFIG_SYS_MCF_SYNCR must be updated to
 * match the new clock speed. Max cpu_clk is 150 MHz.
 */
#define CONFIG_SYS_CLK			100000000
#define CONFIG_SYS_MCF_SYNCR 	(MCF_SYNCR_MFD_4X | MCF_SYNCR_RFD_DIV1)

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
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in internal SRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
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

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_CINV | CF_CACR_INVI)
#define CONFIG_SYS_CACHE_ACR0		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_CENB | CF_CACR_CINV | \
					 CF_CACR_DISD | CF_CACR_INVI | \
					 CF_CACR_CEIB | CF_CACR_DCM | \
					 CF_CACR_EUSP)

/* Chip Select 0  : Boot Flash */
#define CONFIG_SYS_CS0_BASE	0xFFE00000
#define CONFIG_SYS_CS0_MASK	0x001F0001
#define CONFIG_SYS_CS0_CTRL	0x00001980

/* Chip Select 1 : External SRAM */
#define CONFIG_SYS_CS1_BASE	0x30000000
#define CONFIG_SYS_CS1_MASK	0x00070001
#define CONFIG_SYS_CS1_CTRL	0x00001900

#endif				/* _M5271EVB_H */
