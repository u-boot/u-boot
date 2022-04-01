/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Motorola MC5272C3 board.
 *
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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

#define CONFIG_SYS_UART_PORT		(0)

#define CONFIG_WATCHDOG_TIMEOUT 10000	/* timeout in milliseconds */

#undef CONFIG_MONITOR_IS_IN_RAM	/* define if monitor is started from a pre-loader */

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text);

#ifdef CONFIG_MCFFEC
#	define CONFIG_SYS_DISCOVER_PHY
/* If CONFIG_SYS_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CONFIG_SYS_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	endif			/* CONFIG_SYS_DISCOVER_PHY */
#endif

#ifdef CONFIG_MCFFEC
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#endif				/* CONFIG_MCFFEC */

#define CONFIG_HOSTNAME		"M5272C3"
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

#define CONFIG_SYS_CLK			66000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define CONFIG_SYS_MBAR		0x10000000	/* Register Base Addrs */
#define CONFIG_SYS_SCR			0x0003
#define CONFIG_SYS_SPR			0xffff

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in internal SRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		4	/* SDRAM size in MB */
#define CONFIG_SYS_FLASH_BASE		0xffe00000

#define CONFIG_SYS_MONITOR_LEN		0x20000
#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))

/*
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI
#	define CONFIG_SYS_FLASH_SIZE		0x800000	/* Max size that the board might have */
#	define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CONFIG_SYS_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */

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

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CONFIG_SYS_PACNT		0x00000000
#define CONFIG_SYS_PADDR		0x0000
#define CONFIG_SYS_PADAT		0x0000
#define CONFIG_SYS_PBCNT		0x55554155	/* Ethernet/UART configuration */
#define CONFIG_SYS_PBDDR		0x0000
#define CONFIG_SYS_PBDAT		0x0000
#define CONFIG_SYS_PDCNT		0x00000000
#endif				/* _M5272C3_H */
