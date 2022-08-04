/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Sentec Cobra Board.
 *
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 */

/* ---
 * Version: U-Boot 1.0.0 - initial release for Sentec COBRA5272 board
 * Date: 2004-03-29
 * Author: Florian Schlote
 *
 * For a description of configuration options please refer also to the
 * general u-boot-1.x.x/README file
 * ---
 */

/* ---
 * board/config.h - configuration options, board specific
 * ---
 */

#ifndef _CONFIG_COBRA5272_H
#define _CONFIG_COBRA5272_H

/* ---
 * Defines processor clock - important for correct timings concerning serial
 * interface etc.
 * ---
 */

#define CONFIG_SYS_CLK			66000000
#define CONFIG_SYS_SDRAM_SIZE		16		/* SDRAM size in MB */

/* ---
 * Define baudrate for UART1 (console output, tftp, ...)
 * default value of CONFIG_BAUDRATE for Sentec board: 19200 baud
 * CONFIG_SYS_BAUDRATE_TABLE defines values that can be selected in u-boot command
 * interface
 * ---
 */

#define CONFIG_SYS_UART_PORT		(0)

/* ---
 * set "#if 0" to "#if 1" if (Hardware)-WATCHDOG should be enabled & change
 * timeout acc. to your needs
 * #define CONFIG_WATCHDOG_TIMEOUT x , x is timeout in milliseconds, e. g. 10000
 * for 10 sec
 * ---
 */

#if 0
#define CONFIG_WATCHDOG_TIMEOUT 10000	/* timeout in milliseconds */
#endif

/* ---
 * CONFIG_MONITOR_IS_IN_RAM defines if u-boot is started from a different
 * bootloader residing in flash ('chainloading'); if you want to use
 * chainloading or want to compile a u-boot binary that can be loaded into
 * RAM via BDM set
 *	"#if 0" to "#if 1"
 * You will need a first stage bootloader then, e. g. colilo or a working BDM
 * cable (Background Debug Mode)
 *
 * Setting #if 0: u-boot will start from flash and relocate itself to RAM
 *
 * Please do not forget to modify the setting of CONFIG_SYS_TEXT_BASE
 * in board/cobra5272/config.mk accordingly (#if 0: 0xffe00000; #if 1: 0x20000)
 *
 * ---
 */

#if 0
#define CONFIG_MONITOR_IS_IN_RAM /* monitor is started from a preloader */
#endif

/* ---
 * Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 * ---
 */

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text);

/*
 *-----------------------------------------------------------------------------
 * Define user parameters that have to be customized most likely
 *-----------------------------------------------------------------------------
 */

/*AUTOBOOT settings - booting images automatically by u-boot after power on*/

/* The following settings will be contained in the environment block ; if you
want to use a neutral environment all those settings can be manually set in
u-boot: 'set' command */

#if 0

enter a valid image address in flash */

/* User network settings */

#define CONFIG_IPADDR 192.168.100.2		/* default board IP address */
#define CONFIG_SERVERIP 192.168.100.1	/* default tftp server IP address */

#endif

/*---*/

/*
 *-----------------------------------------------------------------------------
 * End of user parameters to be customized
 *-----------------------------------------------------------------------------
 */

/* ---
 * Defines memory range for test
 * ---
 */

/* ---
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * ---
 */

/* ---
 * Base register address
 * ---
 */

#define CONFIG_SYS_MBAR		0x10000000	/* Register Base Addrs */

/* ---
 * System Conf. Reg. & System Protection Reg.
 * ---
 */

#define CONFIG_SYS_SCR			0x0003
#define CONFIG_SYS_SPR			0xffff

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in internal SRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in internal SRAM	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000

/*
 *-------------------------------------------------------------------------
 * RAM SIZE (is defined above)
 *-----------------------------------------------------------------------
 */

/* #define CONFIG_SYS_SDRAM_SIZE		16 */

/*
 *-----------------------------------------------------------------------
 */

#define CONFIG_SYS_FLASH_BASE		0xffe00000

#define CONFIG_SYS_MONITOR_LEN		0x20000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

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
 * LED config
 */
#define	LED_STAT_0	0xffff /*all LEDs off*/
#define	LED_STAT_1	0xfffe
#define	LED_STAT_2	0xfffd
#define	LED_STAT_3	0xfffb
#define	LED_STAT_4	0xfff7
#define	LED_STAT_5	0xffef
#define	LED_STAT_6	0xffdf
#define	LED_STAT_7	0xff00 /*all LEDs on*/

/*-----------------------------------------------------------------------
 * Port configuration (GPIO)
 */
#define CONFIG_SYS_PACNT		0x00000000		/* PortA control reg.: All pins are external
GPIO*/
#define CONFIG_SYS_PADDR		0x00FF			/* PortA direction reg.: PA7 to PA0 are outputs
(1^=output, 0^=input) */
#define CONFIG_SYS_PADAT		LED_STAT_0		/* PortA value reg.: Turn all LED off */
#define CONFIG_SYS_PBCNT		0x55554155		/* PortB control reg.: Ethernet/UART
configuration */
#define CONFIG_SYS_PBDDR		0x0000			/* PortB direction: All pins configured as inputs */
#define CONFIG_SYS_PBDAT		0x0000			/* PortB value reg. */
#define CONFIG_SYS_PDCNT		0x00000000		/* PortD control reg. */

#endif	/* _CONFIG_COBRA5272_H */
