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
#define CONFIG_IDENT_STRING     " $Name:  $"

#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_DU405		1	/* ...on a DU405 board		*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R      1       /* call misc_init_r()           */

#define CONFIG_SYS_CLK_FREQ	25000000 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND	"bootm fff00000"

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

#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_IDE
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_RTC_MC146818		/* BQ3285 is MC146818 compatible*/
#define CFG_RTC_REG_BASE_ADDR	 0xF0000080 /* RTC Base Address		*/

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

#define CFG_EXT_SERIAL_CLOCK	11059200  /* use external serial clock	*/

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_LOAD_ADDR	0x100000	/* default load address */
#define CFG_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CFG_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_AUTO	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_PCI_BOOTDELAY	0	/* enable pci bootdelay variable*/

#define CFG_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh	*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0404	/* PCI Device ID: CPCI-ISER4	*/
#define CFG_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS	0xff000001	/* 16MB, enable hard-wired to 1 */
#define CFG_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/
#define CFG_PCI_PTM2LA	0xffe00000	/* point to flash		*/
#define CFG_PCI_PTM2MS	0xffe00001	/* 2MB, enable			*/
#define CFG_PCI_PTM2PCI 0x04000000	/* Host: use this pci address	*/

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#undef	CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#undef	CONFIG_IDE_RESET		/* no reset for ide supported	*/

#define CFG_IDE_MAXBUS		1		/* max. 1 IDE busses	*/
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*1) /* max. 1 drives per IDE bus */

#define CFG_ATA_BASE_ADDR	0xF0100000
#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O			*/
#define CFG_ATA_REG_OFFSET	0x0000	/* Offset for normal register accesses	*/
#define CFG_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFFD0000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 192 kB for Monitor	*/
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
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
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

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC08) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08		*/
#define CFG_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CFG_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has	*/
					/* 16 byte page write mode using*/
					/* last 4 bits of the address	*/
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE

#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x000	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x400	/* 1024 bytes may be used for env vars */
				   /* total size of a CAT24WC08 is 1024 bytes */

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

#define FLASH_BASE0_PRELIM	0xFF800000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0xFFC00000	/* FLASH bank #1	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

#define FLASH0_BA	0xFFC00000	    /* FLASH 0 Base Address		*/
#define FLASH1_BA	0xFF800000	    /* FLASH 1 Base Address		*/
#define CAN_BA		0xF0000000	    /* CAN Base Address			*/
#define DUART_BA	0xF0300000	    /* DUART Base Address		*/
#define CF_BA		0xF0100000	    /* CompactFlash Base Address	*/
#define SRAM_BA		0xF0200000	    /* SRAM Base Address		*/
#define DURAG_IO_BA	0xF0400000	    /* DURAG Bus IO Base Address	*/
#define DURAG_MEM_BA	0xF0500000	    /* DURAG Bus Mem Base Address	*/

#define FPGA_MODE_REG	(DUART_BA+0x80)	    /* FPGA Mode Register		*/

/* Memory Bank 0 (Flash Bank 0) initialization					*/
#define CFG_EBC_PB0AP	0x92015480
#define CFG_EBC_PB0CR	FLASH0_BA | 0x5A000 /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (Flash Bank 1) initialization					*/
#define CFG_EBC_PB1AP	0x92015480
#define CFG_EBC_PB1CR	FLASH1_BA | 0x5A000 /* BAS=0xFF8,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 2 (CAN0) initialization						*/
#define CFG_EBC_PB2AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB2CR	CAN_BA | 0x18000    /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (DUART) initialization						*/
#define CFG_EBC_PB3AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB3CR	DUART_BA | 0x18000  /* BAS=0xF03,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 4 (CompactFlash IDE) initialization				*/
#define CFG_EBC_PB4AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB4CR	CF_BA | 0x1A000	    /* BAS=0xF01,BS=1MB,BU=R/W,BW=16bit */

/* Memory Bank 5 (SRAM) initialization						*/
#define CFG_EBC_PB5AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB5CR	SRAM_BA | 0x1A000   /* BAS=0xF02,BS=1MB,BU=R/W,BW=16bit */

/* Memory Bank 6 (DURAG Bus IO Space) initialization				*/
#define CFG_EBC_PB6AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB6CR	DURAG_IO_BA | 0x18000 /* BAS=0xF04,BS=1MB,BU=R/W,BW=8bit*/

/* Memory Bank 7 (DURAG Bus Mem Space) initialization				*/
#define CFG_EBC_PB7AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB7CR	DURAG_MEM_BA | 0x18000 /* BAS=0xF05,BS=1MB,BU=R/W,BW=8bit */


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM	  1

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000

#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/
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
