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
#define CONFIG_W7OLMC		1		/* ...specifically an LMC	*/

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f	*/
#define	CONFIG_MISC_INIT_F	1		/* and misc_init_f()		*/
#define	CONFIG_MISC_INIT_R	1		/* and misc_init_r()		*/

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
#undef CONFIG_SYS_LOADS_BAUD_CHANGE			/* disallow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1		/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0		/* PHY address			*/
#define CONFIG_NET_MULTI

#define CONFIG_RTC_M48T35A	1		/* ST Electronics M48 timekeeper */

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

#undef CONFIG_WATCHDOG				/* watchdog disabled		*/
#define CONFIG_HW_WATCHDOG			/* HW Watchdog, board specific	*/

#define	CONFIG_SPD_EEPROM			/* SPD EEPROM for SDRAM param.	*/
#define CONFIG_SPDDRAM_SILENT			/* No output if spd fails	*/
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"Wave7Optics> " /* Monitor Command Prompt	*/
#undef  CONFIG_SYS_HUSH_PARSER				/* No hush parse for U-Boot       */
#ifdef  CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#endif
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size	*/
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM		*/

#undef  CONFIG_SYS_EXT_SERIAL_CLOCK			/* external serial clock */
#define CONFIG_SYS_405_UART_ERRATA_59			/* 405GP/CR Rev. D silicon */
#define CONFIG_SYS_BASE_BAUD		384000


/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{9600}

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address		*/
#define CONFIG_SYS_EXTBDINFO		1		/* use extended board_info (bd_t) */

#define CONFIG_SYS_HZ			1000		/* decrementer freq: 1 ms ticks */

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
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014		/* PCI Vendor ID: IBM		*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0156		/* PCI Device ID: 405GP		*/
#define CONFIG_SYS_PCI_PTM1LA		0x00000000	/* point to sdram		*/
#define CONFIG_SYS_PCI_PTM1MS		0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CONFIG_SYS_PCI_PTM1PCI		0x00000000      /* Host: use this pci address   */
#define CONFIG_SYS_PCI_PTM2LA		0x00000000	/* disabled			*/
#define CONFIG_SYS_PCI_PTM2MS		0x00000000	/* disabled			*/
#define CONFIG_SYS_PCI_PTM2PCI		0x00000000      /* Host: use this pci address   */

/*-----------------------------------------------------------------------
 * Set up values for external bus controller
 * used by cpu_init.c
 *-----------------------------------------------------------------------
 */
 /* Don't use PerWE instead of PCI_INT ( these functions share a pin ) */
#undef CONFIG_USE_PERWE

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM        1

/* bank 0 is boot flash */
/* BME=0,TWT=6,CSN=1,OEN=1,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=1,PEN=0 */
#define CONFIG_SYS_W7O_EBC_PB0AP   0x03050440
/* BAS=0xFFE,BS=0x1(2MB),BU=0x3(R/W),BW=0x0(8 bits) */
#define CONFIG_SYS_W7O_EBC_PB0CR   0xFFE38000

/* bank 1 is main flash */
/* BME=0,TWT=11,CSN=1,OEN=1,WBN=0,WBF=0,TH=1,RE=0,SOR=0,BEM=1,PEN=0 */
#define CONFIG_SYS_EBC_PB1AP   0x05850240
/* BAS=0xF00,BS=0x7(128MB),BU=0x3(R/W),BW=0x10(32 bits) */
#define CONFIG_SYS_EBC_PB1CR   0xF00FC000

/* bank 2 is RTC/NVRAM */
/* BME=0,TWT=6,CSN=0,OEN=0,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=1,PEN=0 */
#define CONFIG_SYS_EBC_PB2AP   0x03000440
/* BAS=0xFC0,BS=0x0(1MB),BU=0x3(R/W),BW=0x0(8 bits) */
#define CONFIG_SYS_EBC_PB2CR   0xFC018000

/* bank 3 is FPGA 0 */
/* BME=0,TWT=4,CSN=0,OEN=0,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=0,PEN=0 */
#define CONFIG_SYS_EBC_PB3AP   0x02000400
/* BAS=0xFD0,BS=0x0(1MB),BU=0x3(R/W),BW=0x1(16 bits) */
#define CONFIG_SYS_EBC_PB3CR   0xFD01A000

/* bank 4 is FPGA 1 */
/* BME=,TWT=,CSN=,OEN=,WBN=,WBF=,TH=,RE=,SOR=,BEM=,PEN= */
#define CONFIG_SYS_EBC_PB4AP   0x02000400
/* BAS=,BS=,BU=0x3(R/W),BW=0x0(8 bits) */
#define CONFIG_SYS_EBC_PB4CR   0xFD11A000

/* bank 5 is FPGA 2 */
/* BME=,TWT=,CSN=,OEN=,WBN=,WBF=,TH=,RE=,SOR=,BEM=,PEN= */
#define CONFIG_SYS_EBC_PB5AP   0x02000400
/* BAS=,BS=,BU=0x3(R/W),BW=0x1(16 bits) */
#define CONFIG_SYS_EBC_PB5CR   0xFD21A000

/* bank 6 is unused */
/* PB6AP = 0 */
#define CONFIG_SYS_EBC_PB6AP   0x00000000
/* PB6CR = 0 */
#define CONFIG_SYS_EBC_PB6CR   0x00000000

/* bank 7 is LED register */
/* BME=0,TWT=6,CSN=1,OEN=1,WBN=0,WBF=0,TH=2,RE=0,SOR=0,BEM=1,PEN=0 */
#define CONFIG_SYS_W7O_EBC_PB7AP   0x03050440
/* BAS=0xFE0,BS=0x0(1MB),BU=0x3(R/W),BW=0x2(32 bits) */
#define CONFIG_SYS_W7O_EBC_PB7CR   0xFE01C000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFFC0000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
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
#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* max number of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	256		/* max number of sec on 1 chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000		/* Timeout, Flash Erase, in ms	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Timeout, Flash Write, in ms	*/
#define CONFIG_SYS_FLASH_PROTECTION	1		/* Use real Flash protection	*/

#if 1 /* Use NVRAM for environment variables */
/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CONFIG_ENV_IS_IN_NVRAM	1		/* use NVRAM for env vars	*/
#define CONFIG_SYS_NVRAM_BASE_ADDR	0xfc000000	/* NVRAM base address		*/
#define CONFIG_SYS_NVRAM_SIZE		(32*1024)	/* NVRAM size			*/
#define CONFIG_ENV_SIZE		0x1000		/* Size of Environment vars	*/
/*define CONFIG_ENV_ADDR		 \
	(CONFIG_SYS_NVRAM_BASE_ADDR+CONFIG_SYS_NVRAM_SIZE-CONFIG_ENV_SIZE) Env  */
#define CONFIG_ENV_ADDR		CONFIG_SYS_NVRAM_BASE_ADDR

#else /* Use Boot Flash for environment variables */
/*-----------------------------------------------------------------------
 * Flash EEPROM for environment
 */
#define CONFIG_ENV_IS_IN_FLASH 1
#define CONFIG_ENV_OFFSET		0x00040000	/* Offset of Environment Sector */
#define CONFIG_ENV_SIZE		0x10000		/* Total Size of env. sector	*/

#define CONFIG_ENV_SECT_SIZE	0x10000		/* see README - env sec tot sze */
#endif

/*-----------------------------------------------------------------------
 * I2C EEPROM (CAT24WC08) for environment
 */
#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM CAT28WC08		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4	/* The Catalyst CAT24WC08 has	*/
					/* 16 byte page write mode using*/
					/* last	4 bits of the address	*/
#define CONFIG_SYS_I2C_MULTI_EEPROMS
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
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in RAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CONFIG_SYS_INIT_RAM_END	CONFIG_SYS_OCM_DATA_SIZE /* End of used area in RAM	*/
#define CONFIG_SYS_GBL_DATA_SIZE	64		/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET


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
#define CONFIG_SYS_FPGA_IMAGE_LEN	0x80000		/* 512KB FPGA image		*/
#define CONFIG_NUM_FPGAS	3		/* Number of FPGAs on board	*/
#define CONFIG_MAX_FPGAS	6		/* Maximum number of FPGAs	*/
#define CONFIG_FPGAS_BASE	0xFD000000L	/* Base address of FPGAs	*/
#define CONFIG_FPGAS_BANK_SIZE	0x00100000L	/* FPGAs' mmap bank size	*/

#endif	/* __CONFIG_H */
