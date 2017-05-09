/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#define CONFIG_PIP405		1	/* ...on a PIP405 board		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFF80000

/***********************************************************
 * Clock
 ***********************************************************/
#define CONFIG_SYS_CLK_FREQ	33000000 /* external frequency to pll	*/

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
#define CONFIG_CMD_IDE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_FDC
#define CONFIG_SCSI
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SAVES

/**************************************************************
 * I2C Stuff:
 * the PIP405 is equiped with an Atmel 24C128/256 EEPROM at address
 * 0x53.
 * Caution: on the same bus is the SPD (Serial Presens Detect
 * EEPROM of the SDRAM
 * The Atmel EEPROM uses 16Bit addressing.
 ***************************************************************/
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		50000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x53
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_ENV_IS_IN_EEPROM    1       /* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET          0x000   /* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE            0x800   /* 2 kBytes may be used for env vars */

#undef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 6	/* The Atmel 24C128/256 has	*/
					/* 64 byte page write mode using*/
					/* last	6 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

/***************************************************************
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 ***************************************************************/
#define SPD_EEPROM_ADDRESS      0x50

#define CONFIG_BOARD_EARLY_INIT_R

/**************************************************************
 * Environment definitions
 **************************************************************/

/* autoboot (do NOT change this set environment variable "bootdelay" to -1 instead) */
/* #define CONFIG_BOOT_RETRY_TIME	-10	/XXX* feature is available but not enabled */

#define CONFIG_BOOTCOMMAND	"diskboot 400000 0:1; bootm" /* autoboot command		*/
#define CONFIG_BOOTARGS		"console=ttyS0,9600 root=/dev/hda5" /* boot arguments */

#define CONFIG_IPADDR		10.0.0.100
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_PREBOOT
/***************************************************************
 * defines if an overwrite_console function exists
 *************************************************************/
/***************************************************************
 * defines if the overwrite_console should be stored in the
 * environment
 **************************************************************/

/**************************************************************
 * loads config
 *************************************************************/
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MISC_INIT_R
/***********************************************************
 * Miscellaneous configurable options
 **********************************************************/
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 1 ... 12 MB in DRAM	*/

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef	CONFIG_SYS_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CONFIG_SYS_BASE_BAUD       691200

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR		0x400000	/* default load address */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0              /* configure ar pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* configure as pci-host	*/
					/* resource configuration	*/
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x0000	/* PCI Vendor ID: to-do!!!	*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0000	/* PCI Device ID: to-do!!!	*/
#define CONFIG_SYS_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CONFIG_SYS_PCI_PTM1MS	0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CONFIG_SYS_PCI_PTM2LA	0x00000000	/* disabled			*/
#define CONFIG_SYS_PCI_PTM2MS	0x00000000	/* disabled			*/
#define CONFIG_SYS_PCI_PTM2PCI 0x00000000      /* Host: use this pci address   */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFF80000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserve 1024 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_UPDATE_FLASH_SIZE
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER

#define CONFIG_FLASH_SHOW_PROGRESS	45

#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256

/*
 * Init Memory Controller:
 */
#define FLASH_MAX_SIZE		0x00800000		/* 8MByte max */
#define FLASH_BASE_PRELIM	0xFF800000  /* open the flash CS */
/* Size: 0=1MB, 1=2MB, 2=4MB, 3=8MB, 4=16MB, 5=32MB, 6=64MB, 7=128MB */
#define FLASH_SIZE_PRELIM	 3  /* maximal flash FLASH size bank #0	*/

/* Configuration Port location */
#define CONFIG_PORT_ADDR	0xF4000000
#define MULTI_PURPOSE_SOCKET_ADDR 0xF8000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in On Chip SRAM)
 */
#define CONFIG_SYS_TEMP_STACK_OCM	1
#define CONFIG_SYS_OCM_DATA_ADDR	0xF0000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR	/* inside of On Chip SRAM    */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE	/* Size of On Chip SRAM	       */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/***********************************************************************
 * External peripheral base address
 ***********************************************************************/
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS 0xE8000000

/***********************************************************************
 * Last Stage Init
 ***********************************************************************/
#define CONFIG_LAST_STAGE_INIT
/************************************************************
 * Ethernet Stuff
 ***********************************************************/
#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address			*/
/************************************************************
 * RTC
 ***********************************************************/
#define CONFIG_RTC_MC146818
#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/************************************************************
 * IDE/ATA stuff
 ************************************************************/
#define CONFIG_SYS_IDE_MAXBUS		2   /* max. 2 IDE busses	*/
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_ISA_IO_BASE_ADDRESS /* base address */
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x01F0	/* ide0 offste */
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x0170	/* ide1 offset */
#define CONFIG_SYS_ATA_DATA_OFFSET	0	/* data reg offset	*/
#define CONFIG_SYS_ATA_REG_OFFSET	0	/* reg offset */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x200	/* alternate register offset */

#undef	CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#define CONFIG_IDE_RESET		/* reset for ide supported...	*/
#define CONFIG_IDE_RESET_ROUTINE	/* with a special reset function */
#define CONFIG_SUPPORT_VFAT

/************************************************************
 * ATAPI support (experimental)
 ************************************************************/
#define CONFIG_ATAPI			/* enable ATAPI Support */

/************************************************************
 * SCSI support (experimental) only SYM53C8xx supported
 ************************************************************/
#define CONFIG_SCSI_SYM53C8XX
#define CONFIG_SYS_SCSI_MAX_LUN	8	/* number of supported LUNs */
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	7	/* maximum SCSI ID (0..6) */
#define CONFIG_SYS_SCSI_MAX_DEVICE	CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN /* maximum Target devices */
#define CONFIG_SYS_SCSI_SPIN_UP_TIME	2

/************************************************************
 * Disk-On-Chip configuration
 ************************************************************/
#define CONFIG_SYS_MAX_DOC_DEVICE	1	/* Max number of DOC devices		*/
#define CONFIG_SYS_DOC_SHORT_TIMEOUT
#define CONFIG_SYS_DOC_SUPPORT_2000
#define CONFIG_SYS_DOC_SUPPORT_MILLENNIUM

/************************************************************
 * DISK Partition support
 ************************************************************/

/************************************************************
 * Video support
 ************************************************************/
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_ONBOARD		/* Video controller is on-board */

/************************************************************
 * USB support
 ************************************************************/
#define CONFIG_USB_UHCI

/* Enable needed helper functions */

/************************************************************
 * Debug support
 ************************************************************/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/************************************************************
 * support BZIP2 compression
 ************************************************************/
#define CONFIG_BZIP2		1

#endif	/* __CONFIG_H */
