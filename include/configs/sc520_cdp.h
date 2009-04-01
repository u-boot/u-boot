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

#define CONFIG_SKIP_RELOCATE_UBOOT

#define GRUSS_TESTING
/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_X86		1	/* This is a X86 CPU		*/
#define CONFIG_SYS_SC520	1	/* Include support for AMD SC520 */
#define CONFIG_ALI152X		1	/* Include support for Ali 152x SIO */

#define CONFIG_SYS_SDRAM_PRECHARGE_DELAY 6     /* 6T */
#define CONFIG_SYS_SDRAM_REFRESH_RATE    78    /* 7.8uS (choices are 7.8, 15.6, 31.2 or 62.5uS) */
#define CONFIG_SYS_SDRAM_RAS_CAS_DELAY   3     /* 3T */

/* define at most one of these */
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_2T
#define CONFIG_SYS_SDRAM_CAS_LATENCY_3T

#define CONFIG_SYS_SC520_HIGH_SPEED    0       /* 100 or 133MHz */
#undef  CONFIG_SYS_SC520_RESET                 /* use SC520 MMCR's to reset cpu */
#undef  CONFIG_SYS_SC520_TIMER                 /* use SC520 swtimers */
#define CONFIG_SYS_GENERIC_TIMER       1       /* use the i8254 PIT timers */
#undef  CONFIG_SYS_TSC_TIMER                   /* use the Pentium TSC timers */
#define  CONFIG_SYS_USE_SIO_UART       0       /* prefer the uarts on the SIO to those
					 * in the SC520 on the CDP */
#define CONFIG_SYS_PCAT_INTERRUPTS
#define CONFIG_SYS_NUM_IRQS		16

#define CONFIG_SYS_STACK_SIZE          0x8000  /* Size of bootloader stack */

#define CONFIG_SHOW_BOOT_PROGRESS 1
#define CONFIG_LAST_STAGE_INIT    1

/*
 * Size of malloc() pool
 */
#define CONFIG_MALLOC_SIZE	(CONFIG_ENV_SIZE + 128*1024)

#define CONFIG_BAUDRATE		9600

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

#define CONFIG_CMD_PCI
#ifndef GRUSS_TESTING
#define CONFIG_CMD_SATA
#else
#undef CONFIG_CMD_SATA
#endif
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_NET
#define CONFIG_CMD_EEPROM

#define CONFIG_BOOTDELAY	15
#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 console=ttyS0,9600"
/* #define CONFIG_BOOTCOMMAND	"bootm 38000000" */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"boot > "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x01000000	/* 1 ... 16 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ			1024		/* incrementer freq: 1kHz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 4 banks of DRAM */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

#define CONFIG_SPI_EEPROM      /* Support for SPI EEPROMs (AT25128) */
#define CONFIG_MW_EEPROM       /* Support for MicroWire EEPROMs (AT93LC46) */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Environment in EEPROM */
#define CONFIG_ENV_IS_IN_EEPROM   1
#define CONFIG_SPI
#define CONFIG_ENV_SIZE	       0x4000	/* Total Size of Environment EEPROM 16k is SPI is used or 128 bytes if MW is used*/
#define CONFIG_ENV_OFFSET         0
#define CONFIG_SYS_SC520_CDP_USE_SPI  /* Store configuration in the SPI part */
#undef CONFIG_SYS_SC520_CDP_USE_MW    /* Store configuration in the MicroWire part */
#define CONFIG_SPI_X 1

/*
 * JFFS2 partitions
 */
/* No command line, one static partition, whole device */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
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
*SATA/Native Stuff
************************************************************/
#ifndef GRUSS_TESTING
#define CONFIG_SYS_SATA_MAXBUS         2       /*Max Sata buses supported */
#define CONFIG_SYS_SATA_DEVS_PER_BUS   2      /*Max no. of devices per bus/port */
#define CONFIG_SYS_SATA_MAX_DEVICE     (CONFIG_SYS_SATA_MAXBUS* CONFIG_SYS_SATA_DEVS_PER_BUS)
#define CONFIG_ATA_PIIX		1       /*Supports ata_piix driver */
#else
#undef CONFIG_SYS_SATA_MAXBUS
#undef CONFIG_SYS_SATA_DEVS_PER_BUS
#undef CONFIG_SYS_SATA_MAX_DEVICE
#undef CONFIG_ATA_PIIX
#endif


/************************************************************
 * DISK Partition support
 ************************************************************/
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION /* Experimental */

/************************************************************
 * Video/Keyboard support
 ************************************************************/
#ifndef GRUSS_TESTING
#define CONFIG_VIDEO			/* To enable video controller support */
#else
#undef CONFIG_VIDEO
#endif
#define CONFIG_I8042_KBD
#define CONFIG_SYS_ISA_IO 0

/************************************************************
 * RTC
 ***********************************************************/
#define CONFIG_RTC_MC146818
#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * PCI stuff
 */
#ifndef GRUSS_TESTING
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_PCI_PNP                            /* pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW

#define	CONFIG_SYS_FIRST_PCI_IRQ   10
#define	CONFIG_SYS_SECOND_PCI_IRQ  9
#define CONFIG_SYS_THIRD_PCI_IRQ   11
#define	CONFIG_SYS_FORTH_PCI_IRQ   15
#else
#undef CONFIG_PCI
#undef CONFIG_PCI_PNP
#undef CONFIG_PCI_SCAN_SHOW
#endif


#endif	/* __CONFIG_H */
