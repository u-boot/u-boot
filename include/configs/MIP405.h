/*
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/***********************************************************
 * High Level Configuration Options
 * (easy to change)
 ***********************************************************/
#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_MIP405		1	/* ...on a MIP405 board		*/
/***********************************************************
 * Note that it may also be a MIP405T board which is a subset of the
 * MIP405
 ***********************************************************/
/***********************************************************
 * WARNING:
 * CONFIG_BOOT_PCI is only used for first boot-up and should
 * NOT be enabled for production bootloader
 ***********************************************************/
/*#define        CONFIG_BOOT_PCI         1*/
/***********************************************************
 * Clock
 ***********************************************************/
#define CONFIG_SYS_CLK_FREQ	33000000 /* external frequency to pll   */


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
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_BSP

#if !defined(CONFIG_MIP405T)
    #define CONFIG_CMD_USB
    #define CONFIG_CMD_DOC
#endif


#define CFG_NAND_LEGACY

#define	 CFG_HUSH_PARSER
#define	 CFG_PROMPT_HUSH_PS2 "> "
/**************************************************************
 * I2C Stuff:
 * the MIP405 is equiped with an Atmel 24C128/256 EEPROM at address
 * 0x53.
 * The Atmel EEPROM uses 16Bit addressing.
 ***************************************************************/

#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CFG_I2C_SPEED		50000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x53	/* EEPROM 24C128/256		*/
#define CFG_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"    */
#undef CFG_I2C_EEPROM_ADDR_OVERFLOW
#define CFG_EEPROM_PAGE_WRITE_BITS 6	/* The Atmel 24C128/256 has	*/
					/* 64 byte page write mode using*/
					/* last	6 bits of the address	*/
#define CFG_EEPROM_PAGE_WRITE_ENABLE	/* enable Page write */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */


#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x00000	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x00800	/* 2k bytes may be used for env vars */

/***************************************************************
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 ***************************************************************/
/*#define SDRAM_EEPROM_WRITE_ADDRESS	0xA0
#define SDRAM_EEPROM_READ_ADDRESS	0xA1
*/
/**************************************************************
 * Environment definitions
 **************************************************************/
#define CONFIG_BAUDRATE		9600	/* STD Baudrate */
#define CONFIG_BOOTDELAY	5
/* autoboot (do NOT change this set environment variable "bootdelay" to -1 instead) */
/* #define CONFIG_BOOT_RETRY_TIME	-10	/XXX* feature is available but not enabled */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check console even if bootdelay = 0 */

#define CONFIG_BOOTCOMMAND	"diskboot 400000 0:1; bootm" /* autoboot command		*/
#define CONFIG_BOOTARGS		"console=ttyS0,9600 root=/dev/hda5" /* boot arguments */

#define CONFIG_IPADDR		10.0.0.100
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_PREBOOT
/***************************************************************
 * defines if the console is stored in the environment
 ***************************************************************/
#define CFG_CONSOLE_IS_IN_ENV	/* stdin, stdout and stderr are in evironment */
/***************************************************************
 * defines if an overwrite_console function exists
 *************************************************************/
#define CFG_CONSOLE_OVERWRITE_ROUTINE
#define CFG_CONSOLE_INFO_QUIET
/***************************************************************
 * defines if the overwrite_console should be stored in the
 * environment
 **************************************************************/
#undef CFG_CONSOLE_ENV_OVERWRITE

/**************************************************************
 * loads config
 *************************************************************/
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MISC_INIT_R
/***********************************************************
 * Miscellaneous configurable options
 **********************************************************/
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

#define CFG_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 1 ... 12 MB in DRAM	*/

#undef	CFG_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CFG_BASE_BAUD       916667

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_LOAD_ADDR	0x400000	/* default load address */
#define CFG_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0              /* configure as pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* configure as pci-host	*/
#define CONFIG_PCI_PNP			/* pci plug-and-play		*/
					/* resource configuration	*/
#define CFG_PCI_SUBSYS_VENDORID 0x0000	/* PCI Vendor ID: to-do!!!	*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0000	/* PCI Device ID: to-do!!!	*/
#define CFG_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS	0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CFG_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CFG_PCI_PTM2LA	0x00000000	/* disabled			*/
#define CFG_PCI_PTM2MS	0x00000000	/* disabled			*/
#define CFG_PCI_PTM2PCI 0x00000000      /* Host: use this pci address   */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFF80000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Monitor	*/
#define CFG_MALLOC_LEN		(1024 * 1024)	/* Reserve 1024 kB for malloc()	*/

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

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=mip405-0"
#define MTDPARTS_DEFAULT	"mtdparts=mip405-0:-(jffs2)"
*/

/*-----------------------------------------------------------------------
 * Logbuffer Configuration
 */
#undef CONFIG_LOGBUFFER		/* supported but not enabled */
/*-----------------------------------------------------------------------
 * Bootcountlimit Configuration
 */
#undef CONFIG_BOOTCOUNT_LIMIT	/* supported but not enabled */

/*-----------------------------------------------------------------------
 * POST Configuration
 */
#if 0 /* enable this if POST is desired (is supported but not enabled) */
#define CONFIG_POST		(CFG_POST_MEMORY	| \
				 CFG_POST_CPU		| \
				 CFG_POST_RTC		| \
				 CFG_POST_I2C)

#endif
/*
 * Init Memory Controller:
 */
#define FLASH_MAX_SIZE		0x00800000		/* 8MByte max */
#define FLASH_BASE_PRELIM	0xFF800000  /* open the flash CS */
/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define FLASH_SIZE_PRELIM	 3  /* maximal flash FLASH size bank #0	*/

#define CONFIG_BOARD_EARLY_INIT_F 1

/* Peripheral Bus Mapping */
#define PER_PLD_ADDR		0xF4000000 /* smallest window is 1MByte 0x10 0000*/
#define PER_UART0_ADDR		0xF4100000 /* smallest window is 1MByte 0x10 0000*/
#define PER_UART1_ADDR		0xF4200000 /* smallest window is 1MByte 0x10 0000*/

#define MULTI_PURPOSE_SOCKET_ADDR 0xF8000000
#define CONFIG_PORT_ADDR	PER_PLD_ADDR + 5


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in On Chip SRAM)
 */
#define CFG_TEMP_STACK_OCM      1
#define CFG_OCM_DATA_ADDR	0xF0000000
#define CFG_OCM_DATA_SIZE	0x1000
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR	/* inside of On Chip SRAM    */
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE	/* End of On Chip SRAM	       */
#define CFG_GBL_DATA_SIZE	64		/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
/* reserve some memory for POST and BOOT limit info */
#define CFG_INIT_SP_OFFSET	(CFG_GBL_DATA_OFFSET - 32)

#ifdef  CONFIG_POST		/* reserve one word for POST Info */
#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 4)
#endif

#ifdef CONFIG_BOOTCOUNT_LIMIT /* reserve 2 word for bootcount limit */
#define CFG_BOOTCOUNT_ADDR (CFG_GBL_DATA_OFFSET - 12)
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/


/***********************************************************************
 * External peripheral base address
 ***********************************************************************/
#define CFG_ISA_IO_BASE_ADDRESS 0xE8000000

/***********************************************************************
 * Last Stage Init
 ***********************************************************************/
#define CONFIG_LAST_STAGE_INIT
/************************************************************
 * Ethernet Stuff
 ***********************************************************/
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address			*/
#define CONFIG_PHY_RESET_DELAY	300	/* Intel LXT971A needs this */
#define CONFIG_PHY_CMD_DELAY	40	/* Intel LXT971A needs this */
/************************************************************
 * RTC
 ***********************************************************/
#define CONFIG_RTC_MC146818
#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/************************************************************
 * IDE/ATA stuff
 ************************************************************/
#if defined(CONFIG_MIP405T)
#define CFG_IDE_MAXBUS		1   /* MIP405T has only one IDE bus	*/
#else
#define CFG_IDE_MAXBUS		2   /* max. 2 IDE busses	*/
#endif

#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

#define CFG_ATA_BASE_ADDR	CFG_ISA_IO_BASE_ADDRESS /* base address */
#define CFG_ATA_IDE0_OFFSET	0x01F0		/* ide0 offste */
#define CFG_ATA_IDE1_OFFSET	0x0170		/* ide1 offset */
#define CFG_ATA_DATA_OFFSET	0		/* data reg offset	*/
#define CFG_ATA_REG_OFFSET	0		/* reg offset */
#define CFG_ATA_ALT_OFFSET	0x200		/* alternate register offset */

#undef	CONFIG_IDE_8xx_DIRECT      /* no pcmcia interface required */
#undef	CONFIG_IDE_LED	       /* no led for ide supported     */
#define CONFIG_IDE_RESET       /* reset for ide supported...	*/
#define CONFIG_IDE_RESET_ROUTINE /* with a special reset function */
#define CONFIG_SUPPORT_VFAT
/************************************************************
 * ATAPI support (experimental)
 ************************************************************/
#define CONFIG_ATAPI			/* enable ATAPI Support */

/************************************************************
 * DISK Partition support
 ************************************************************/
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION /* Experimental */

/************************************************************
 * Disk-On-Chip configuration
 ************************************************************/
#define CFG_MAX_DOC_DEVICE	1	/* Max number of DOC devices		*/
#define CFG_DOC_SHORT_TIMEOUT
#define CFG_DOC_SUPPORT_2000
#define CFG_DOC_SUPPORT_MILLENNIUM
/************************************************************
 * Keyboard support
 ************************************************************/
#undef CONFIG_ISA_KEYBOARD

/************************************************************
 * Video support
 ************************************************************/
#define CONFIG_VIDEO			/*To enable video controller support */
#define CONFIG_VIDEO_CT69000
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_SW_CURSOR
#undef CONFIG_VIDEO_ONBOARD
/************************************************************
 * USB support EXPERIMENTAL
 ************************************************************/
#if !defined(CONFIG_MIP405T)
#define CONFIG_USB_UHCI
#define CONFIG_USB_KEYBOARD
#define CONFIG_USB_STORAGE

/* Enable needed helper functions */
#define CFG_DEVICE_DEREGISTER		/* needs device_deregister */
#endif
/************************************************************
 * Debug support
 ************************************************************/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/************************************************************
 * support BZIP2 compression
 ************************************************************/
#define CONFIG_BZIP2		1

/************************************************************
 * Ident
 ************************************************************/

#define VERSION_TAG "released"
#if !defined(CONFIG_MIP405T)
#define CONFIG_ISO_STRING "MEV-10072-001"
#else
#define CONFIG_ISO_STRING "MEV-10082-001"
#endif

#if !defined(CONFIG_BOOT_PCI)
#define CONFIG_IDENT_STRING "\n(c) 2003 by MPL AG Switzerland, " CONFIG_ISO_STRING " " VERSION_TAG
#else
#define CONFIG_IDENT_STRING "\n(c) 2003 by MPL AG Switzerland, PCI_BOOT Version"
#endif


#endif	/* __CONFIG_H */
