/*
 * Configuation settings for the Sentec Cobra Board.
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

/* ---
 * Version: U-boot 1.0.0 - initial release for Sentec COBRA5272 board
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
 * Define processor
 * possible values for Sentec board: only Coldfire M5272 processor supported
 * (please do not change)
 * ---
 */

#define CONFIG_MCF52x2			/* define processor family */
#define CONFIG_M5272			/* define processor type */

/* ---
 * Defines processor clock - important for correct timings concerning serial
 * interface etc.
 * CFG_HZ gives unit: 1000 -> 1 Hz ^= 1000 ms
 * ---
 */

#define CFG_HZ			1000
#define CFG_CLK			66000000
#define CFG_SDRAM_SIZE		16		/* SDRAM size in MB */

/* ---
 * Enable use of Ethernet
 * ---
 */

#define FEC_ENET

/* ---
 * Define baudrate for UART1 (console output, tftp, ...)
 * default value of CONFIG_BAUDRATE for Sentec board: 19200 baud
 * CFG_BAUDRATE_TABLE defines values that can be selected in u-boot command
 * interface
 * ---
 */

#define CONFIG_BAUDRATE		19200
#define CFG_BAUDRATE_TABLE { 9600 , 19200 , 38400 , 57600, 115200 }

/* ---
 * set "#if 0" to "#if 1" if (Hardware)-WATCHDOG should be enabled & change
 * timeout acc. to your needs
 * #define CONFIG_WATCHDOG_TIMEOUT x , x is timeout in milliseconds, e. g. 10000
 * for 10 sec
 * ---
 */

#if 0
#define CONFIG_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT 10000	/* timeout in milliseconds */
#endif

/* ---
 * CONFIG_MONITOR_IS_IN_RAM defines if u-boot is started from a different
 * bootloader residing in flash ('chainloading'); if you want to use
 * chainloading or want to compile a u-boot binary that can be loaded into
 * RAM via BDM set
 * 	"#if 0" to "#if 1"
 * You will need a first stage bootloader then, e. g. colilo or a working BDM
 * cable (Background Debug Mode)
 *
 * Setting #if 0: u-boot will start from flash and relocate itself to RAM
 *
 * Please do not forget to modify the setting of TEXT_BASE
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

/* ---
 * Define which commmands should be available at u-boot command prompt
 * ---
 */

#define CONFIG_COMMANDS	 ( CONFIG_CMD_DFL | CFG_CMD_PING & ~(CFG_CMD_LOADS | \
CFG_CMD_LOADB) | CFG_CMD_MII)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 *-----------------------------------------------------------------------------
 * Define user parameters that have to be customized most likely
 *-----------------------------------------------------------------------------
 */

/*AUTOBOOT settings - booting images automatically by u-boot after power on*/

#define CONFIG_BOOTDELAY	5		/* used for autoboot, delay in
seconds u-boot will wait before starting defined (auto-)boot command, setting
to -1 disables delay, setting to 0 will too prevent access to u-boot command
interface: u-boot then has to reflashed */


/* The following settings will be contained in the environment block ; if you
want to use a neutral environment all those settings can be manually set in
u-boot: 'set' command */

#if 0

#define CONFIG_BOOTCOMMAND	"bootm 0xffe80000"	/*Autoboto command, please
enter a valid image address in flash */

#define CONFIG_BOOTARGS		" "			/* default bootargs that are
considered during boot */

/* User network settings */

#define CONFIG_ETHADDR 00:00:00:00:00:09	/* default ethernet MAC addr. */
#define CONFIG_IPADDR 192.168.100.2		/* default board IP address */
#define CONFIG_SERVERIP 192.168.100.1	/* default tftp server IP address */

#endif

#define CFG_PROMPT		"COBRA > "	/* Layout of u-boot prompt*/

#define CFG_LOAD_ADDR		0x20000		/*Defines default RAM address
from which user programs will be started */

/*---*/

#define CFG_LONGHELP				/* undef to save memory		*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

/*
 *-----------------------------------------------------------------------------
 * End of user parameters to be customized
 *-----------------------------------------------------------------------------
 */

/* ---
 * Defines memory range for test
 * ---
 */

#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

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

#define CFG_MBAR		0x10000000	/* Register Base Addrs */

/* ---
 * System Conf. Reg. & System Protection Reg.
 * ---
 */

#define CFG_SCR			0x0003;
#define CFG_SPR			0xffff;

/* ---
 * Ethernet settings
 * ---
 */

#define CFG_DISCOVER_PHY
#define CFG_ENET_BD_BASE	0x780000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in internal SRAM)
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

/*
 *-------------------------------------------------------------------------
 * RAM SIZE (is defined above)
 *-----------------------------------------------------------------------
 */

/* #define CFG_SDRAM_SIZE		16 */

/*
 *-----------------------------------------------------------------------
 */

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
#define CFG_FLASH_ERASE_TOUT	1000	/* flash timeout */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 *
 * Please refer also to Motorola Coldfire user manual - Chapter XXX
 * <http://e-www.motorola.com/files/dsp/doc/ref_manual/MCF5272UM.pdf>
 */
#define CFG_BR0_PRELIM		0xFFE00201
#define CFG_OR0_PRELIM		0xFFE00014

#define CFG_BR1_PRELIM		0
#define CFG_OR1_PRELIM		0

#define CFG_BR2_PRELIM		0
#define CFG_OR2_PRELIM		0

#define CFG_BR3_PRELIM		0
#define CFG_OR3_PRELIM		0

#define CFG_BR4_PRELIM		0
#define CFG_OR4_PRELIM		0

#define CFG_BR5_PRELIM		0
#define CFG_OR5_PRELIM		0

#define CFG_BR6_PRELIM		0
#define CFG_OR6_PRELIM		0

#define CFG_BR7_PRELIM		0x00000701
#define CFG_OR7_PRELIM		0xFF00007C

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
#define CFG_PACNT		0x00000000		/* PortA control reg.: All pins are external
GPIO*/
#define CFG_PADDR		0x00FF			/* PortA direction reg.: PA7 to PA0 are outputs
(1^=output, 0^=input) */
#define CFG_PADAT		LED_STAT_0		/* PortA value reg.: Turn all LED off */
#define CFG_PBCNT		0x55554155		/* PortB control reg.: Ethernet/UART
configuration */
#define CFG_PBDDR		0x0000			/* PortB direction: All pins configured as inputs */
#define CFG_PBDAT		0x0000			/* PortB value reg. */
#define CFG_PDCNT		0x00000000		/* PortD control reg. */

#endif	/* _CONFIG_COBRA5272_H */
