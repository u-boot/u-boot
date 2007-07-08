/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
 *
 * (C) Copyright 2001-2004
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_PCI405		1	/* ...on a PCI405 board		*/

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

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/

#define CONFIG_RTC_M48T35A	1		/* ST Electronics M48 timekeeper */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PCI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DATE
#define CONFIG_CMD_I2C
#define CONFIG_CMD_BSP
#define CONFIG_CMD_EEPROM


#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

#define CONFIG_PRAM		2048	/* reserve 2 MB "protected RAM" */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#endif

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

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#undef	CFG_EXT_SERIAL_CLOCK	       /* no external serial clock used */
#define CFG_IGNORE_405_UART_ERRATA_59	/* ignore ppc405gp errata #59	*/
#define CFG_BASE_BAUD	    691200

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_LOAD_ADDR	0x100000	/* default load address */
#define CFG_EXTBDINFO	1		/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

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
#define CONFIG_PCI_HOST PCI_HOST_ADAPTER /* select pci host function	*/
#undef	CONFIG_PCI_PNP			/* no pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CFG_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh	*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0407	/* PCI Device ID: PCI-405	*/
#define CFG_PCI_CLASSCODE	0x0280	/* PCI Class Code: Network/Other*/
#define CFG_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS	0xff000001	/* 16MB, enable hard-wired to 1 */
#define CFG_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/

#if 0 /* test-only */
#define CFG_PCI_PTM2LA	0xffc00000	/* point to flash		*/
#define CFG_PCI_PTM2MS	0xffc00001	/* 4MB, enable			*/
#define CFG_PCI_PTM2PCI 0x04000000	/* Host: use this pci address	*/
#else
#define CFG_PCI_PTM2LA	0xef600000	/* point to internal regs	*/
#define CFG_PCI_PTM2MS	0xffe00001	/* 2MB, enable			*/
#define CFG_PCI_PTM2PCI 0x00000000	/* Host: use this pci address	*/
#endif

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFFD0000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
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

#if 0 /* Use NVRAM for environment variables */
/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_ENV_IS_IN_NVRAM	1	/* use NVRAM for environment vars	*/
#define CFG_ENV_SIZE		0x0ff8		/* Size of Environment vars	*/
#define CFG_ENV_ADDR		\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-(CFG_ENV_SIZE+8))	/* Env	*/

#else /* Use EEPROM for environment variables */

#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CFG_ENV_OFFSET		0x000	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x400	/* 1024 bytes may be used for env vars*/
				   /* total size of a CAT24WC08 is 1024 bytes */
#endif

#define CFG_NVRAM_BASE_ADDR	0xf0200000		/* NVRAM base address	*/
#define CFG_NVRAM_SIZE		(32*1024)		/* NVRAM size		*/

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC16) for environment
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

#define FLASH_BASE0_PRELIM	0xFFE00000	/* FLASH bank #0	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */

/* Memory Bank 0 (Flash Bank 0) initialization					*/
#define CFG_EBC_PB0AP		0x92015480
#define CFG_EBC_PB0CR		0xFFC5A000  /* BAS=0xFFC,BS=4MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (NVRAM/RTC) initialization					*/
#define CFG_EBC_PB1AP		0x01005280  /* TWT=2,WBN=1,WBF=1,TH=1,SOR=1	*/
#define CFG_EBC_PB1CR		0xF0218000  /* BAS=0xF02,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 2 (CAN0, 1) initialization					*/
#define CFG_EBC_PB2AP		0x010053C0  /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
/*#define CFG_EBC_PB2AP		  0x038056C0  / * BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB2CR		0xF0018000  /* BAS=0xF00,BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 3 (FPGA internal) initialization					*/
#define CFG_EBC_PB3AP		0x010053C0  /* BWT=2,WBN=1,WBF=1,TH=1,RE=1,SOR=1,BEM=1 */
#define CFG_EBC_PB3CR		0xF041C000  /* BAS=0xF01,BS=1MB,BU=R/W,BW=32bit */
#define CFG_FPGA_BASE_ADDR	0xF0400000

/*-----------------------------------------------------------------------
 * FPGA stuff
 */
/* FPGA internal regs */
#define CFG_FPGA_MODE		0x00
#define CFG_FPGA_STATUS		0x02
#define CFG_FPGA_TS		0x04
#define CFG_FPGA_TS_LOW		0x06
#define CFG_FPGA_TS_CAP0	0x10
#define CFG_FPGA_TS_CAP0_LOW	0x12
#define CFG_FPGA_TS_CAP1	0x14
#define CFG_FPGA_TS_CAP1_LOW	0x16
#define CFG_FPGA_TS_CAP2	0x18
#define CFG_FPGA_TS_CAP2_LOW	0x1a
#define CFG_FPGA_TS_CAP3	0x1c
#define CFG_FPGA_TS_CAP3_LOW	0x1e

/* FPGA Mode Reg */
#define CFG_FPGA_MODE_CF_RESET	0x0001
#define CFG_FPGA_MODE_TS_IRQ_ENABLE 0x0100
#define CFG_FPGA_MODE_TS_IRQ_CLEAR  0x1000
#define CFG_FPGA_MODE_TS_CLEAR	0x2000

/* FPGA Status Reg */
#define CFG_FPGA_STATUS_DIP0	0x0001
#define CFG_FPGA_STATUS_DIP1	0x0002
#define CFG_FPGA_STATUS_DIP2	0x0004
#define CFG_FPGA_STATUS_FLASH	0x0008
#define CFG_FPGA_STATUS_TS_IRQ	0x1000

#define CFG_FPGA_SPARTAN2	1	    /* using Xilinx Spartan 2 now    */
#define CFG_FPGA_MAX_SIZE	32*1024	    /* 32kByte is enough for XC2S15  */

/* FPGA program pin configuration */
#define CFG_FPGA_PRG		0x04000000  /* FPGA program pin (ppc output) */
#define CFG_FPGA_CLK		0x02000000  /* FPGA clk pin (ppc output)     */
#define CFG_FPGA_DATA		0x01000000  /* FPGA data pin (ppc output)    */
#define CFG_FPGA_INIT		0x00400000  /* FPGA init pin (ppc input)     */
#define CFG_FPGA_DONE		0x00800000  /* FPGA done pin (ppc input)     */
/* new INIT and DONE pins since board revision 1.2 (for PPC405GPr support)   */
#define CFG_FPGA_INIT_V12	0x00008000  /* FPGA init pin (ppc input)     */
#define CFG_FPGA_DONE_V12	0x00010000  /* FPGA done pin (ppc input)     */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */
#if 0 /* test-only */
#define CFG_INIT_DCACHE_CS	7	/* use cs # 7 for data cache memory    */
#define CFG_INIT_RAM_ADDR	0x40000000  /* use data cache		       */
#define CFG_INIT_RAM_END	0x2000	/* End of used area in RAM	       */
#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET
#else
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
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */
