/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com
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

#define CONFIG_405GP		1		/* This is a PPC405GP CPU	*/
#define CONFIG_4xx		1		/* ...member of PPC405 family	*/
#define CONFIG_W7O		1		/* ...on a Wave 7 Optics board	*/
#define CONFIG_W7OLMG		1		/* ...specifically an LMG	*/

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f	*/
#define	CONFIG_MISC_INIT_F	1		/* and misc_init_f()		*/

#define CONFIG_SYS_CLK_FREQ	33333333	/* external frequency to pll	*/

#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	3		/* autoboot after 3 seconds	*/

#if 1
#define CONFIG_BOOTCOMMAND	"bootvx"	/* VxWorks boot command		*/
#else
#define CONFIG_BOOTCOMMAND	"bootp"		/* autoboot command		*/
#endif

#undef CONFIG_BOOTARGS

#define CONFIG_LOADADDR		F0080000

#define CONFIG_ETHADDR		00:06:0D:00:00:00 /* Default, overridden at boot	*/
#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_IPADDR		192.168.1.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_SERVERIP		192.168.1.2

#define CONFIG_LOADS_ECHO	1		/* echo on for serial download	*/
#undef CFG_LOADS_BAUD_CHANGE			/* disallow baudrate change	*/

#define CONFIG_MII		1		/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0		/* PHY address			*/

#define CONFIG_RTC_M48T35A	1		/* ST Electronics M48 timekeeper */
#define CONFIG_DTT_LM75     1                /* ON Semi's LM75 */
#define CONFIG_DTT_SENSORS  {2, 4}           /* Sensor addresses */
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_LOW_TEMP	-30
#define CFG_DTT_HYSTERESIS	3


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
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_BSP
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_DTT


#undef CONFIG_WATCHDOG				/* watchdog disabled		*/
#define CONFIG_HW_WATCHDOG			/* HW Watchdog, board specific	*/

#define	CONFIG_SPD_EEPROM			/* SPD EEPROM for SDRAM param.	*/
#define CONFIG_SPDDRAM_SILENT			/* No output if spd fails	*/
/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		*/
#define CFG_PROMPT		"Wave7Optics> " /* Monitor Command Prompt	*/
#undef  CFG_HUSH_PARSER				/* No hush parse for U-Boot       */
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2     "> "
#endif
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM		*/

#undef  CFG_EXT_SERIAL_CLOCK			/* external serial clock */
#define CFG_405_UART_ERRATA_59			/* 405GP/CR Rev. D silicon */
#define CFG_BASE_BAUD		384000


/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE	{9600}

#define CFG_CLKS_IN_HZ		1		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0x100000	/* default load address		*/
#define CFG_EXTBDINFO		1		/* use extended board_info (bd_t) */

#define CFG_HZ			1000		/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER	0		/* configure as pci adapter	*/
#define PCI_HOST_FORCE		1		/* configure as pci host	*/
#define PCI_HOST_AUTO		2		/* detected via arbiter enable	*/

#define CONFIG_PCI				/* include pci support		*/
#define CONFIG_PCI_HOST		PCI_HOST_AUTO	/* select pci host function	*/
#define CONFIG_PCI_PNP				/* pci plug-and-play		*/
/* resource configuration	*/
#define CFG_PCI_SUBSYS_VENDORID 0x1014		/* PCI Vendor ID: IBM		*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0156		/* PCI Device ID: 405GP		*/
#define CFG_PCI_PTM1LA		0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS		0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CFG_PCI_PTM1PCI 	0x00000000      /* Host: use this pci address   */
#define CFG_PCI_PTM2LA		0x00000000	/* disabled		*/
#define CFG_PCI_PTM2MS		0x00000000	/* disabled		*/
#define CFG_PCI_PTM2PCI 	0x00000000      /* Host: use this pci address   */

/*-----------------------------------------------------------------------
 * Set up values for external bus controller
 * used by cpu_init.c
 *-----------------------------------------------------------------------
 */
 /* use PerWE instead of PCI_INT ( these functions share a pin ) */
#define CONFIG_USE_PERWE 1

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CFG_TEMP_STACK_OCM        1

/* bank 0 is boot flash */
/* BME=0,TWT=6,CSN=1,OEN=1,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=1,PEN=0 */
#define CFG_W7O_EBC_PB0AP   0x03050440
/* BAS=0xFFE,BS=0x1(2MB),BU=0x3(R/W),BW=0x0(8 bits) */
#define CFG_W7O_EBC_PB0CR   0xFFE38000

/* bank 1 is main flash */
/* BME=0,TWT=9,CSN=1,OEN=1,WBN=0,WBF=0,TH=1,RE=0,SOR=0,BEM=1,PEN=0 */
#define CFG_EBC_PB1AP   0x04850240
/* BAS=0xF00,BS=0x7(128MB),BU=0x3(R/W),BW=0x10(32 bits) */
#define CFG_EBC_PB1CR   0xF00FC000

/* bank 2 is RTC/NVRAM */
/* BME=0,TWT=6,CSN=0,OEN=0,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=1,PEN=0 */
#define CFG_EBC_PB2AP   0x03000440
/* BAS=0xFC0,BS=0x0(1MB),BU=0x3(R/W),BW=0x0(8 bits) */
#define CFG_EBC_PB2CR   0xFC018000

/* bank 3 is FPGA 0 */
/* BME=0,TWT=4,CSN=0,OEN=0,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=0,PEN=0 */
#define CFG_EBC_PB3AP   0x02000400
/* BAS=0xFD0,BS=0x0(1MB),BU=0x3(R/W),BW=0x1(16 bits) */
#define CFG_EBC_PB3CR   0xFD01A000

/* bank 4 is SAM 8 bit range */
/* BME=,TWT=,CSN=,OEN=,WBN=,WBF=,TH=,RE=,SOR=,BEM=,PEN= */
#define CFG_EBC_PB4AP   0x02840380
/* BAS=,BS=,BU=0x3(R/W),BW=0x0(8 bits) */
#define CFG_EBC_PB4CR   0xFE878000

/* bank 5 is SAM 16 bit range */
/* BME=0,TWT=10,CSN=2,OEN=0,WBN=0,WBF=0,TH=6,RE=1,SOR=1,BEM=0,PEN=0 */
#define CFG_EBC_PB5AP   0x05040d80
/* BAS=,BS=,BU=0x3(R/W),BW=0x1(16 bits) */
#define CFG_EBC_PB5CR   0xFD87A000

/* bank 6 is unused */
/* pb6ap = 0 */
#define CFG_EBC_PB6AP   0x00000000
/* pb6cr = 0 */
#define CFG_EBC_PB6CR   0x00000000

/* bank 7 is LED register */
/* BME=0,TWT=6,CSN=1,OEN=1,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=1,PEN=0 */
#define CFG_W7O_EBC_PB7AP   0x03050440
/* BAS=0xFE0,BS=0x0(1MB),BU=0x3(R/W),BW=0x2(32 bits) */
#define CFG_W7O_EBC_PB7CR   0xFE01C000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFFC0000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 196 kB for Monitor	*/
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
#define CFG_MAX_FLASH_BANKS	2		/* max number of memory banks	*/
#define CFG_MAX_FLASH_SECT	256		/* max number of sec on 1 chip	*/

#define CFG_FLASH_ERASE_TOUT	120000		/* Timeout, Flash Erase, in ms	*/
#define CFG_FLASH_WRITE_TOUT	500		/* Timeout, Flash Write, in ms	*/
#define CFG_FLASH_PROTECTION	1		/* Use real Flash protection	*/

#if 1 /* Use NVRAM for environment variables */
/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_ENV_IS_IN_NVRAM	1		/* use NVRAM for env vars	*/
#define CFG_NVRAM_BASE_ADDR	0xfc000000	/* NVRAM base address		*/
#define CFG_NVRAM_SIZE		(32*1024)	/* NVRAM size			*/
#define CFG_ENV_SIZE		0x1000		/* Size of Environment vars	*/
/*define CFG_ENV_ADDR		 \
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE) Env  */
#define CFG_ENV_ADDR		CFG_NVRAM_BASE_ADDR

#else /* Use Boot Flash for environment variables */
/*-----------------------------------------------------------------------
 * Flash EEPROM for environment
 */
#define CFG_ENV_IS_IN_FLASH 1
#define CFG_ENV_OFFSET		0x00040000	/* Offset of Environment Sector */
#define CFG_ENV_SIZE		0x10000		/* Total Size of env. sector	*/

#define CFG_ENV_SECT_SIZE	0x10000		/* see README - env sec tot sze */
#endif

/*-----------------------------------------------------------------------
 * I2C EEPROM (ATMEL 24C04N)
 */
#define CONFIG_HARD_I2C		1		/* Hardware assisted I2C	*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	0x50		/* EEPROM ATMEL 24C04N		*/
#define CFG_I2C_EEPROM_ADDR_LEN	1		/* Bytes of address		*/
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_I2C_MULTI_EEPROMS
/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS      0x50	/* XXX conflicting address!!! XXX */

/*
 * Init Memory Controller:
 */
#define FLASH_BASE0_PRELIM	0xFFE00000	/* FLASH bank #0		*/
#define FLASH_BASE1_PRELIM	0xF0000000	/* FLASH bank #1		*/

/* On Chip Memory location */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in RAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/
#define CFG_GBL_DATA_SIZE	64		/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02		/* Software reboot		*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use	*/
#endif

/*
 * FPGA(s) configuration
 */
#define CFG_FPGA_IMAGE_LEN	0x80000		/* 512KB FPGA image		*/
#define CONFIG_NUM_FPGAS	1		/* Number of FPGAs on board	*/
#define CONFIG_MAX_FPGAS	6		/* Maximum number of FPGAs	*/
#define CONFIG_FPGAS_BASE	0xFD000000L	/* Base address of FPGAs	*/
#define CONFIG_FPGAS_BANK_SIZE	0x00100000L	/* FPGAs' mmap bank size	*/

#endif	/* __CONFIG_H */
