/*
 * (C) Copyright 2001
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
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

#define CONFIG_405CR		1	/* This is a PPC405CR CPU	*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_CANBT		1	/* ...on a CANBT board		*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/

#define CONFIG_SYS_CLK_FREQ	25000000 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	1	/* autoboot after 1 seconds	*/

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw console=ttyS0,115200; "	\
	"bootm ffe00000 ffe80000"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#undef	CONFIG_PCI_PNP			/* no pci plug-and-play		*/

#define CONFIG_PHY_ADDR		0	/* PHY address			*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_IRQ
#define CONFIG_CMD_EEPROM	   

#undef CONFIG_CMD_NET


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_EXT_SERIAL_CLOCK	14745600 /* use external serial clock	*/

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_LOAD_ADDR	0x100000	/* default load address */
#define CFG_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFFE0000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(128 * 1024)	/* Reserve 128 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_WORD_SIZE	unsigned short	/* flash word size (width)	*/
#define CFG_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CFG_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CFG_FLASH_READ0		0x0000	/* 0 is standard			*/
#define CFG_FLASH_READ1		0x0001	/* 1 is standard			*/
#define CFG_FLASH_READ2		0x0002	/* 2 is standard			*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#if 0 /* Use FLASH for environment variables */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x00010000	/* Offset of Environment Sector */
#define CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector	*/

#define CFG_ENV_SECT_SIZE	0x10000 /* see README - env sector total size	*/

#else /* Use EEPROM for environment variables */

#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x000	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x400	/* 1024 bytes may be used for env vars */
				   /* total size of a CAT24WC08 is 1024 bytes */
#endif

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC08) for environment
 */
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08		*/
#define CFG_I2C_EEPROM_ADDR_LEN 1	/* bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		8192	/* For AMCC 405 CPUs			*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFFC00000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0		/* FLASH bank #1	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash Bank 0) initialization					*/
#define CFG_EBC_PB0AP		0x92015480
#define CFG_EBC_PB0CR		0xFFC5A000  /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (CAN/USB) initialization					*/
#define CFG_EBC_PB1AP		0x010053C0  /* enable Ready, BEM=1		*/
#define CFG_EBC_PB1CR		0xF0018000  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 2 (Misc-IO/LEDs) initialization					*/
#define CFG_EBC_PB2AP		0x000004c0  /* no Ready, BEM=1			*/
#define CFG_EBC_PB2CR		0xF0118000  /* BAS=0xF01,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (CAN Features) initialization					*/
#define CFG_EBC_PB3AP		0x80000040  /* no Ready, BEM=1			*/
#define CFG_EBC_PB3CR		0xF021C000  /* BAS=0xF02,BS=1MB,BU=R/W,BW=32bit */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in RAM)
 */
#define CFG_INIT_RAM_ADDR	0x00ef0000 /* inside of SDRAM			*/
#define CFG_INIT_RAM_END	0x0f00	/* End of used area in RAM	       */
#define CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
