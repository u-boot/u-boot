/*
 * (C) Copyright 2001-2004
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

#define CONFIG_405GP		1	/* This is a PPC405GP CPU	*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_AR405		1	/* ...on a AR405 board		*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/

#define CONFIG_SYS_CLK_FREQ	33000000 /* external frequency to pll	*/

#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#if 1
#define CONFIG_BOOTCOMMAND	"bootm fff00000" /* autoboot command	*/
#else
#define CONFIG_BOOTCOMMAND	"bootp" /* autoboot command		*/
#endif

#if 0
#define CONFIG_BOOTARGS		"root=/dev/nfs "			\
    "ip=192.168.2.176:192.168.2.190:192.168.2.79:255.255.255.0 "	\
    "nfsroot=192.168.2.190:/home/stefan/cpci405/target_ftest4"
#else
#define CONFIG_BOOTARGS		"root=/dev/hda1 "			\
    "ip=192.168.2.176:192.168.2.190:192.168.2.79:255.255.255.0"

#endif

#define CONFIG_PREBOOT                  /* enable preboot variable      */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP  1       /* disable sleep mode in LXT971 */


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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_BSP


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

#define CFG_DEVICE_NULLDEV	1	/* include nulldev device	*/

#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */

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
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_PCI_CONFIG_HOST_BRIDGE 1 /* don't skip host bridge config*/

#define CONFIG_PCI_BOOTDELAY	0	/* enable pci bootdelay variable*/

#define CFG_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh	*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0403	/* PCI Device ID: ARISTO405	*/
#define CFG_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS	0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CFG_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/
#define CFG_PCI_PTM2LA	0xfff00000	/* point to flash		*/
#define CFG_PCI_PTM2MS	0xfff00001	/* 1MB, enable			*/
#define CFG_PCI_PTM2PCI 0x04000000	/* Host: use this pci address	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFFC0000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
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

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0xFFFB0000	/* Address of Environment Sector*/
#define CFG_ENV_SECT_SIZE	0x10000 /* see README - env sector total size	*/
#define CFG_ENV_SIZE		0x04000	        /* Size of Environment 	        */

#define CFG_ENV_ADDR_REDUND     0xFFFA0000
#define CFG_ENV_SIZE_REDUND	CFG_ENV_SIZE

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFFC00000	/* FLASH bank #0	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash Bank 0) initialization					*/
#define CFG_EBC_PB0AP		0x92015480
#define CFG_EBC_PB0CR		0xFFC5A000  /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (CAN0, 1, 2, 3) initialization					*/
#define CFG_EBC_PB1AP		0x01000380  /* enable Ready, BEM=0		*/
#define CFG_EBC_PB1CR		0xF0018000  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 2 (Expension Bus) initialization					*/
#define CFG_EBC_PB2AP		0x01000280  /* disable Ready, BEM=0		*/
#define CFG_EBC_PB2CR		0xF0118000  /* BAS=0xF01,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (16552) initialization						*/
#define CFG_EBC_PB3AP		0x01000380  /* enable Ready, BEM=0		*/
#define CFG_EBC_PB3CR		0xF0218000  /* BAS=0xF02,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 4 (FPGA regs) initialization					*/
#define CFG_EBC_PB4AP		0x01005380  /* enable Ready, BEM=0		*/
#define CFG_EBC_PB4CR		0xF031C000  /* BAS=0xF03,BS=1MB,BU=R/W,BW=32bit */

/* Memory Bank 5 (Flash Bank 1/DUMMY) initialization				*/
#define CFG_EBC_PB5AP		0x92015480
#define CFG_EBC_PB5CR		0xFF85A000  /* BAS=0xFF8,BS=4MB,BU=R/W,BW=16bit */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
#define CFG_INIT_DCACHE_CS	7	/* use cs # 7 for data cache memory    */

#define CFG_INIT_RAM_ADDR	0x40000000  /* use data cache		       */
#define CFG_INIT_RAM_END	0x2000	/* End of used area in RAM	       */
#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
