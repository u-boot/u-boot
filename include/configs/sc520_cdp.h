/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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

#define CONFIG_X86		1	/* This is a X86 CPU		*/
#define CONFIG_SC520		1	/* Include support for AMD SC520 */
#define CONFIG_ALI152X		1	/* Include support for Ali 152x SIO */

#define CFG_SDRAM_PRECHARGE_DELAY 6     /* 6T */
#define CFG_SDRAM_REFRESH_RATE    78    /* 7.8uS (choices are 7.8, 15.6, 31.2 or 62.5uS) */
#define CFG_SDRAM_RAS_CAS_DELAY   3     /* 3T */

/* define at most one of these */
#undef CFG_SDRAM_CAS_LATENCY_2T
#define CFG_SDRAM_CAS_LATENCY_3T

#define CFG_SC520_HIGH_SPEED    0       /* 100 or 133MHz */
#define CFG_RESET_GENERIC       1       /* use tripple-fault to reset cpu */
#undef  CFG_RESET_SC520                 /* use SC520 MMCR's to reset cpu */
#undef  CFG_TIMER_SC520                 /* use SC520 swtimers */
#define CFG_TIMER_GENERIC       1       /* use the i8254 PIT timers */
#undef  CFG_TIMER_TSC                   /* use the Pentium TSC timers */
#define  CFG_USE_SIO_UART       0       /* prefer the uarts on the SIO to those
					 * in the SC520 on the CDP */

#define CFG_STACK_SIZE          0x8000  /* Size of bootloader stack */

#define CONFIG_SHOW_BOOT_PROGRESS 1
#define CONFIG_LAST_STAGE_INIT    1

/*
 * Size of malloc() pool
 */
#define CONFIG_MALLOC_SIZE	(CFG_ENV_SIZE + 128*1024)

#define CONFIG_BAUDRATE		9600

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PCI
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NET
#define CONFIG_CMD_EEPROM

#define CONFIG_BOOTDELAY	15
#define CONFIG_BOOTARGS    	"root=/dev/mtdblock0 console=ttyS0,9600"
/* #define CONFIG_BOOTCOMMAND	"bootm 38000000" */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"boot > "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x01000000	/* 1 ... 16 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ			1024		/* incrementer freq: 1kHz */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 4 banks of DRAM */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	3	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */

#define CONFIG_SPI_EEPROM      /* Support for SPI EEPROMs (AT25128) */
#define CONFIG_MW_EEPROM       /* Support for MicroWire EEPROMs (AT93LC46) */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Environment in EEPROM */
#define CFG_ENV_IS_IN_EEPROM   1
#define CONFIG_SPI
#define CFG_ENV_SIZE	       0x4000	/* Total Size of Environment EEPROM 16k is SPI is used or 128 bytes if MW is used*/
#define CFG_ENV_OFFSET         0
#define CONFIG_SC520_CDP_USE_SPI  /* Store configuration in the SPI part */
#undef CONFIG_SC520_CDP_USE_MW    /* Store configuration in the MicroWire part */
#define CONFIG_SPI_X 1

/*
 * JFFS2 partitions
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=SC520CDP Flash Bank #0"
#define MTDPARTS_DEFAULT	"mtdparts=SC520CDP Flash Bank #0:-(jffs2)"
*/

/*-----------------------------------------------------------------------
 * Device drivers
 */
#define CONFIG_NET_MULTI        /* Multi ethernet cards support */
#define CONFIG_PCNET
#define CONFIG_PCNET_79C973
#define CONFIG_PCNET_79C975
#define PCNET_HAS_PROM         1

/************************************************************
 * IDE/ATA stuff
 ************************************************************/
#define CFG_IDE_MAXBUS		1   /* max. 2 IDE busses	*/
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

#define CFG_ATA_IDE0_OFFSET	0x01F0	/* ide0 offste */
/*#define CFG_ATA_IDE1_OFFSET	0x0170	/###* ide1 offset */
#define CFG_ATA_DATA_OFFSET	0	/* data reg offset	*/
#define CFG_ATA_REG_OFFSET	0	/* reg offset */
#define CFG_ATA_ALT_OFFSET	0x200	/* alternate register offset */
#define CFG_ATA_BASE_ADDR       0

#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#undef  CONFIG_IDE_RESET		/* reset for ide unsupported...	*/
#undef  CONFIG_IDE_RESET_ROUTINE	/* no special reset function */

/************************************************************
*SATA/Native Stuff
************************************************************/
#define CFG_SATA_SUPPORTED      1
#define CFG_SATA_MAXBUS         2       /*Max Sata buses supported */
#define CFG_SATA_DEVS_PER_BUS   2      /*Max no. of devices per bus/port */
#define CFG_SATA_MAXDEVICES     (CFG_SATA_MAXBUS* CFG_SATA_DEVS_PER_BUS)
#define CFG_ATA_PIIX            1       /*Supports ata_piix driver */

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
 * Video/Keyboard support
 ************************************************************/
#define CONFIG_VIDEO			/* To enable video controller support */
#define CONFIG_I8042_KBD
#define CFG_ISA_IO 0

/************************************************************
 * RTC
 ***********************************************************/
#define CONFIG_RTC_MC146818
#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * PCI stuff
 */
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_PCI_PNP                            /* pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW

#define	CFG_FIRST_PCI_IRQ   10
#define	CFG_SECOND_PCI_IRQ  9
#define CFG_THIRD_PCI_IRQ   11
#define	CFG_FORTH_PCI_IRQ   15

#endif	/* __CONFIG_H */
