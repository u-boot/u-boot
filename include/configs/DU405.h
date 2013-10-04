/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_DU405		1	/* ...on a DU405 board		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFD0000

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R      1       /* call misc_init_r()           */

#define CONFIG_SYS_CLK_FREQ	25000000 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND	"bootm fff00000"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_LXT971_NO_SLEEP  1       /* disable sleep mode in LXT971 */
#define CONFIG_RESET_PHY_R      1       /* use reset_phy() to disable phy sleep mode */
#undef  CONFIG_HAS_ETH1

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

#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_EDITENV
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_CONSOLE
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_IDE
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_RTC_MC146818		/* BQ3285 is MC146818 compatible*/
#define CONFIG_SYS_RTC_REG_BASE_ADDR	 0xF0000080 /* RTC Base Address		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
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

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#define CONFIG_SYS_EXT_SERIAL_CLOCK	11059200  /* use external serial clock	*/

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CONFIG_SYS_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#undef	CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#undef	CONFIG_IDE_RESET		/* no reset for ide supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1		/* max. 1 IDE busses	*/
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*1) /* max. 1 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR	0xF0100000
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_REG_OFFSET	0x0000	/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFFD0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 * 1024)	/* Reserve 192 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned short	/* flash word size (width)	*/
#define CONFIG_SYS_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CONFIG_SYS_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CONFIG_SYS_FLASH_READ0		0x0000	/* 0 is standard			*/
#define CONFIG_SYS_FLASH_READ1		0x0001	/* 1 is standard			*/
#define CONFIG_SYS_FLASH_READ2		0x0002	/* 2 is standard			*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC08) for environment
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has	*/
					/* 16 byte page write mode using*/
					/* last 4 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */

#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x000	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x400	/* 1024 bytes may be used for env vars */
				   /* total size of a CAT24WC08 is 1024 bytes */

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
#define CONFIG_SYS_EBC_PB0AP	0x92015480
#define CONFIG_SYS_EBC_PB0CR	FLASH0_BA | 0x5A000 /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (Flash Bank 1) initialization					*/
#define CONFIG_SYS_EBC_PB1AP	0x92015480
#define CONFIG_SYS_EBC_PB1CR	FLASH1_BA | 0x5A000 /* BAS=0xFF8,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 2 (CAN0) initialization						*/
#define CONFIG_SYS_EBC_PB2AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB2CR	CAN_BA | 0x18000    /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (DUART) initialization						*/
#define CONFIG_SYS_EBC_PB3AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB3CR	DUART_BA | 0x18000  /* BAS=0xF03,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 4 (CompactFlash IDE) initialization				*/
#define CONFIG_SYS_EBC_PB4AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB4CR	CF_BA | 0x1A000	    /* BAS=0xF01,BS=1MB,BU=R/W,BW=16bit */

/* Memory Bank 5 (SRAM) initialization						*/
#define CONFIG_SYS_EBC_PB5AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB5CR	SRAM_BA | 0x1A000   /* BAS=0xF02,BS=1MB,BU=R/W,BW=16bit */

/* Memory Bank 6 (DURAG Bus IO Space) initialization				*/
#define CONFIG_SYS_EBC_PB6AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB6CR	DURAG_IO_BA | 0x18000 /* BAS=0xF04,BS=1MB,BU=R/W,BW=8bit*/

/* Memory Bank 7 (DURAG Bus Mem Space) initialization				*/
#define CONFIG_SYS_EBC_PB7AP	0x010053C0   /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB7CR	DURAG_MEM_BA | 0x18000 /* BAS=0xF05,BS=1MB,BU=R/W,BW=8bit */


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	  1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#endif	/* __CONFIG_H */
