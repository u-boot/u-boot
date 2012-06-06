/*
 * (C) Copyright 2001
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_IOP480		1	/* This is a IOP480 CPU		*/
#define CONFIG_DASA_SIM		1	/* ...on a DASA_SIM board	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000
#define CONFIG_SYS_LDSCRIPT	"board/esd/dasa_sim/u-boot.lds"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/

#define CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_CPUCLOCK		66
#define CONFIG_BUSCLOCK		(CONFIG_CPUCLOCK)

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds	*/
#define CONFIG_BOOTCOMMAND	"bootm ffe00000" /* autoboot command	*/

#undef	CONFIG_BOOTARGS

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_IPADDR		10.0.18.222
#define CONFIG_SERVERIP		10.0.18.190


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

#define CONFIG_CMD_BSP


#if 0 /* Does not appear to be used?!  If it is used, needs to be fixed */
#define CONFIG_SOFT_I2C			/* Software I2C support enabled */
#endif
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00df0000 /* inside of SDRAM			*/
#define CONFIG_SYS_INIT_RAM_SIZE	0x0f00	/* Size of used area in RAM	       */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFFC0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 128 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned char	/* flash word size (width)	*/
#define CONFIG_SYS_FLASH_ADDR0		0x0AA9	/* 1st address for flash config cycles	*/
#define CONFIG_SYS_FLASH_ADDR1		0x0556	/* 2nd address for flash config cycles	*/
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CONFIG_SYS_FLASH_READ0		0x0002	/* 0 is standard			*/
#define CONFIG_SYS_FLASH_READ1		0x0000	/* 1 is standard			*/
#define CONFIG_SYS_FLASH_READ2		0x0004	/* 2 is standard			*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x00010000	/* Offset of Environment Sector */
#define CONFIG_ENV_SIZE		0x1000	/* Total Size of Environment Sector	*/

#if 0
#define CONFIG_ENV_SECT_SIZE	0x8000	/* see README - env sector total size	*/
#else
#define CONFIG_ENV_SECT_SIZE	0x10000 /* see README - env sector total size	*/
#endif

/*-----------------------------------------------------------------------
 * PCI stuff
 */
#define CONFIG_PCI			/* include pci support			*/
#undef CONFIG_PCI_PNP

#define CONFIG_NET_MULTI		/* Multi ethernet cards support		*/

#define CONFIG_TULIP

#define CONFIG_SYS_ETH_DEV_FN	     0x0000
#define CONFIG_SYS_ETH_IOBASE	     0x0fff0000
#define CONFIG_SYS_PCI9054_DEV_FN   0x0800
#define CONFIG_SYS_PCI9054_IOBASE   0x0eff0000

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFFE00000	/* FLASH bank #0	*/

#endif	/* __CONFIG_H */
