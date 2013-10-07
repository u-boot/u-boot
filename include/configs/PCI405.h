/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
 *
 * (C) Copyright 2001-2004
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#define CONFIG_PCI405		1	/* ...on a PCI405 board		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFD0000

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r() on init	*/

#define CONFIG_SYS_CLK_FREQ	25000000 /* external frequency to pll	*/

#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	0	/* autoboot after 0 seconds	*/

#undef	CONFIG_BOOTARGS
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"mem_linux=14336k\0"					        \
	"optargs=panic=0\0"					        \
	"ramargs=setenv bootargs mem=$mem_linux root=/dev/ram rw\0"	\
	"addcons=setenv bootargs $bootargs console=ttyS0,$baudrate $optargs\0" \
	""
#define	CONFIG_BOOTCOMMAND      "run ramargs;run addcons;loadpci"

#define CONFIG_PREBOOT                  /* enable preboot variable      */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

#define CONFIG_CMD_PCI
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_BSP
#define CONFIG_CMD_EEPROM

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

#define CONFIG_PRAM		2048	/* reserve 2 MB "protected RAM" */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser	*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_DEVICE_NULLDEV	1	/* include nulldev device	*/

#define CONFIG_SYS_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef	CONFIG_SYS_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CONFIG_SYS_BASE_BAUD	    691200

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#undef CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */

#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure as pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST PCI_HOST_ADAPTER /* select pci host function	*/
#undef	CONFIG_PCI_PNP			/* no pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh	*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0407	/* PCI Device ID: PCI-405	*/
#define CONFIG_SYS_PCI_CLASSCODE	0x0280	/* PCI Class Code: Network/Other*/
#define CONFIG_SYS_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CONFIG_SYS_PCI_PTM1MS	0xff000001	/* 16MB, enable hard-wired to 1 */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/

#define CONFIG_SYS_PCI_PTM2LA	0xef600000	/* point to internal regs	*/
#define CONFIG_SYS_PCI_PTM2MS	0xffe00001	/* 2MB, enable			*/
#define CONFIG_SYS_PCI_PTM2PCI 0x00000000	/* Host: use this pci address	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFFD0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
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
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
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

#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_ENV_OFFSET		0x000	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x400	/* 1024 bytes may be used for env vars*/
				   /* total size of a CAT24WC08 is 1024 bytes */

#define CONFIG_SYS_NVRAM_BASE_ADDR	0xf0200000		/* NVRAM base address	*/
#define CONFIG_SYS_NVRAM_SIZE		(32*1024)		/* NVRAM size		*/

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC16) for environment
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

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFFE00000	/* FLASH bank #0	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash Bank 0) initialization					*/
#define CONFIG_SYS_EBC_PB0AP		0x92015480
#define CONFIG_SYS_EBC_PB0CR		0xFFC5A000  /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (NVRAM/RTC) initialization					*/
#define CONFIG_SYS_EBC_PB1AP		0x01005280  /* TWT=2,WBN=1,WBF=1,TH=1,SOR=1	*/
#define CONFIG_SYS_EBC_PB1CR		0xF0218000  /* BAS=0xF02,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 2 (CAN0, 1) initialization					*/
#define CONFIG_SYS_EBC_PB2AP		0x010053C0  /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
/*#define CONFIG_SYS_EBC_PB2AP		  0x038056C0  / * BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB2CR		0xF0018000  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (FPGA internal) initialization					*/
#define CONFIG_SYS_EBC_PB3AP		0x010053C0  /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CONFIG_SYS_EBC_PB3CR		0xF041C000  /* BAS=0xF01,BS=1MB,BU=R/W,BW=32bit */
#define CONFIG_SYS_FPGA_BASE_ADDR	0xF0400000

/*-----------------------------------------------------------------------
 * FPGA stuff
 */
/* FPGA internal regs */
#define CONFIG_SYS_FPGA_MODE		0x00
#define CONFIG_SYS_FPGA_STATUS		0x02
#define CONFIG_SYS_FPGA_TS		0x04
#define CONFIG_SYS_FPGA_TS_LOW		0x06
#define CONFIG_SYS_FPGA_TS_CAP0	0x10
#define CONFIG_SYS_FPGA_TS_CAP0_LOW	0x12
#define CONFIG_SYS_FPGA_TS_CAP1	0x14
#define CONFIG_SYS_FPGA_TS_CAP1_LOW	0x16
#define CONFIG_SYS_FPGA_TS_CAP2	0x18
#define CONFIG_SYS_FPGA_TS_CAP2_LOW	0x1a
#define CONFIG_SYS_FPGA_TS_CAP3	0x1c
#define CONFIG_SYS_FPGA_TS_CAP3_LOW	0x1e

/* FPGA Mode Reg */
#define CONFIG_SYS_FPGA_MODE_CF_RESET	0x0001
#define CONFIG_SYS_FPGA_MODE_TS_IRQ_ENABLE 0x0100
#define CONFIG_SYS_FPGA_MODE_TS_IRQ_CLEAR  0x1000
#define CONFIG_SYS_FPGA_MODE_TS_CLEAR	0x2000

/* FPGA Status Reg */
#define CONFIG_SYS_FPGA_STATUS_DIP0	0x0001
#define CONFIG_SYS_FPGA_STATUS_DIP1	0x0002
#define CONFIG_SYS_FPGA_STATUS_DIP2	0x0004
#define CONFIG_SYS_FPGA_STATUS_FLASH	0x0008
#define CONFIG_SYS_FPGA_STATUS_TS_IRQ	0x1000

#define CONFIG_SYS_FPGA_SPARTAN2	1	    /* using Xilinx Spartan 2 now    */
#define CONFIG_SYS_FPGA_MAX_SIZE	32*1024	    /* 32kByte is enough for XC2S15  */

/* FPGA program pin configuration */
#define CONFIG_SYS_FPGA_PRG		0x04000000  /* FPGA program pin (ppc output) */
#define CONFIG_SYS_FPGA_CLK		0x02000000  /* FPGA clk pin (ppc output)     */
#define CONFIG_SYS_FPGA_DATA		0x01000000  /* FPGA data pin (ppc output)    */
#define CONFIG_SYS_FPGA_INIT		0x00400000  /* FPGA init pin (ppc input)     */
#define CONFIG_SYS_FPGA_DONE		0x00800000  /* FPGA done pin (ppc input)     */
/* new INIT and DONE pins since board revision 1.2 (for PPC405GPr support)   */
#define CONFIG_SYS_FPGA_INIT_V12	0x00008000  /* FPGA init pin (ppc input)     */
#define CONFIG_SYS_FPGA_DONE_V12	0x00010000  /* FPGA done pin (ppc input)     */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
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
