/*
 * (C) Copyright 2001
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

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_ERIC		1	/* ...on a ERIC board	*/

#define	CONFIG_BOARD_EARLY_INIT_F 1	/* run board_early_init_f() */

#define CONFIG_SYS_CLK_FREQ	33333333 /* external frequency to pll	*/

#if 1
#define CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars	*/
#endif
#if 0
#define CONFIG_ENV_IS_IN_NVRAM	1	/* use NVRAM for environment vars	*/
#endif
#if 0
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use I2C RTC X1240 for environment vars */
#define CONFIG_ENV_OFFSET		0x000	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x800	/* 2048 bytes may be used for env vars */
#endif					/* total size of a X1240 is 2048 bytes */

#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57	/* X1240 has two I2C slave addresses, one for EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* address length for the eeprom */
#define CONFIG_I2C_RTC		1	/* we have a Xicor X1240 RTC */
#define CONFIG_SYS_I2C_RTC_ADDR	0x6F	/*                                and one for RTC */

#ifdef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_EEPROM
#else
#ifdef CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_IN_EEPROM
#else
#ifdef CONFIG_ENV_IS_IN_EEPROM
#undef CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_FLASH
#endif
#endif
#endif

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds	*/

#if 1
#define CONFIG_BOOTCOMMAND	"bootm ffc00000" /* autoboot command	*/
#else
#define CONFIG_BOOTCOMMAND	"bootp" /* autoboot command		*/
#endif

#define CONFIG_BOOTARGS		"console=ttyS0,115200 root=/dev/nfs "	\
				"nfsroot=192.168.1.2:/eric_root_devel "	\
				"ip=192.168.1.22:192.168.1.2"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address			*/
#define CONFIG_NET_MULTI


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
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FLASH


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#undef	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	 CONFIG_SYS_EXT_SERIAL_CLOCK	 14318180

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0              /* configure ar pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_HOST	PCI_HOST_FORCE  /* select pci host function     */
#undef  CONFIG_PCI_PNP			/* no pci plug-and-play         */
					/* resource configuration       */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1743	/* PCI Vendor ID: Peppercon AG	*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0405	/* PCI Device ID: 405GP		*/
#define CONFIG_SYS_PCI_PTM1LA	0xFFFC0000	/* point to flash		*/
#define CONFIG_SYS_PCI_PTM1MS	0xFFFFF001	/* 4kB, enable hard-wired to 1	*/
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CONFIG_SYS_PCI_PTM2LA	0x00000000	/* disabled			*/
#define CONFIG_SYS_PCI_PTM2MS	0x00000000	/* disabled			*/
#define CONFIG_SYS_PCI_PTM2PCI 0x04000000      /* Host: use this pci address   */

/*-----------------------------------------------------------------------
 * External peripheral base address
 *-----------------------------------------------------------------------
 */
/*	Bank 0 - Flash/SRAM	       0xFF000000 16MB	16 Bit */
/*	Bank 1 - NVRAM/RTC	       0xF0000000  1MB	 8 Bit */
/*	Bank 2 - A/D converter	       0xF0100000  1MB	 8 Bit */
/*	Bank 3 - Ethernet PHY Reset    0xF0200000  1MB	 8 Bit */
/*	Bank 4 - PC-MIP PRSNT1#	       0xF0300000  1MB	 8 Bit */
/*	Bank 5 - PC-MIP PRSNT2#	       0xF0400000  1MB	 8 Bit */
/*	Bank 6 - CPU LED0	       0xF0500000  1MB	 8 Bit */
/*	Bank 7 - CPU LED1	       0xF0600000  1MB	 8 Bit */

/* ----------------------------------------------------------------------- */
/*  Memory Bank 0 (Flash) initialization */
/* ----------------------------------------------------------------------- */
#define CS0_AP	0x9B015480
#define CS0_CR	0xFF87A000 /*  BAS=0xFF8,BS=(8MB),BU=0x3(R/W), BW=(16 bits) */
/* ----------------------------------------------------------------------- */
/*  Memory Bank 1 (NVRAM/RTC) initialization */
/* ----------------------------------------------------------------------- */
#define CS1_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS1_CR	0xF0018000 /* BAS=0xF00,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */
 /* ----------------------------------------------------------------------- */
 /*  Memory Bank 2 (A/D converter) initialization */
 /* ----------------------------------------------------------------------- */
#define CS2_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS2_CR	0xF0118000 /* BAS=0xF01,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */
/* ----------------------------------------------------------------------- */
/*  Memory Bank 3 (Ethernet PHY Reset) initialization */
/* ----------------------------------------------------------------------- */
#define CS3_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS3_CR	0xF0218000 /* BAS=0xF01,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */
/* ----------------------------------------------------------------------- */
/*  Memory Bank 4 (PC-MIP PRSNT1#) initialization */
/* ----------------------------------------------------------------------- */
#define CS4_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS4_CR	0xF0318000 /* BAS=0xF01,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */
/* ----------------------------------------------------------------------- */
/*  Memory Bank 5 (PC-MIP PRSNT2#) initialization */
/* ----------------------------------------------------------------------- */
#define CS5_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS5_CR	0xF0418000 /* BAS=0xF01,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */
/* ----------------------------------------------------------------------- */
/*  Memory Bank 6 (CPU LED0) initialization */
/* ----------------------------------------------------------------------- */
#define CS6_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS6_CR	0xF0518000 /* BAS=0xF01,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */
/* ----------------------------------------------------------------------- */
/*  Memory Bank 7 (CPU LED1) initialization */
/* ----------------------------------------------------------------------- */
#define CS7_AP	0x02815480 /* WT=1, OEN=0x1,WBN=0x1,WBF=0x1,TH=0x2,RE=0, */
#define CS7_CR	0xF0618000 /* BAS=0xF01,BS=(1MB),BU=0x3(R/W), BW=(8 bits) */

#define CONFIG_SYS_NVRAM_REG_BASE_ADDR	 0xF0000000
#define CONFIG_SYS_RTC_REG_BASE_ADDR	 (0xF0000000 + 0x7F8)
#define CONFIG_SYS_ADC_REG_BASE_ADDR	 0xF0100000
#define CONFIG_SYS_PHYRES_REG_BASE_ADDR 0xF0200000
#define CONFIG_SYS_PRSNT1_REG_BASE_ADDR 0xF0300000
#define CONFIG_SYS_PRSNT2_REG_BASE_ADDR 0xF0400000
#define CONFIG_SYS_LED0_REG_BASE_ADDR	 0xF0500000
#define CONFIG_SYS_LED1_REG_BASE_ADDR	 0xF0600000


/*  SDRAM CONFIG */
#define CONFIG_SYS_SDRAM_MANUALLY    1
#define CONFIG_SYS_SDRAM_SINGLE_BANK 1

#ifdef CONFIG_SYS_SDRAM_MANUALLY
/*-----------------------------------------------------------------------
 * Set MB0CF for bank 0. (0-32MB) Address Mode 4 since 12x8(2)
 *----------------------------------------------------------------------*/
#define MB0CF	0x00062001 /*  32MB @ 0 */
/*-----------------------------------------------------------------------
 * Set MB1CF for bank 1. (32MB-64MB) Address Mode 4 since 12x8(2)
 *----------------------------------------------------------------------*/
#ifdef CONFIG_SYS_SDRAM_SINGLE_BANK
#define MB1CF	0x0 /*  0MB @ 32MB */
#else
#define MB1CF	0x02062001 /*  32MB @ 32MB */
#endif
/*-----------------------------------------------------------------------
 * Set MB2CF for bank 2. off
 *----------------------------------------------------------------------*/
#define MB2CF	0x0 /*  0MB */
/*-----------------------------------------------------------------------
 * Set MB3CF for bank 3. off
 *----------------------------------------------------------------------*/
#define MB3CF	0x0 /*  0MB */

#define SDTR_100    0x0086400D
#define RTR_100     0x05F0
#define SDTR_66     0x00854006	/* orig U-Boot-wallnut says 0x00854006 */
#define RTR_66      0x03f8

#endif   /* CONFIG_SYS_SDRAM_MANUALLY */


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		32
#define CONFIG_SYS_FLASH_BASE		0xFF800000      /* 8 MByte Flash */
#define CONFIG_SYS_MONITOR_BASE	0xFFFE0000      /* last 128kByte within Flash */
/*#define CONFIG_SYS_MONITOR_LEN		(192 * 1024)*/	/* Reserve 196 kB for Monitor	*/
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)	/* Reserve 128 kB for Monitor	*/
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
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/
#define CONFIG_SYS_FLASH_16BIT		1	/* Rom 16 bit data bus			*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/* BEG ENVIRONNEMENT FLASH */
#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE       (128*1024)

#if 0  /* force ENV to be NOT embedded */
#define CONFIG_ENV_ADDR            0xfffa0000
#else  /* force ENV to be embedded */
#define	CONFIG_ENV_SIZE		(2 * 1024) /* Total Size of Environment Sector 2k */
#define CONFIG_ENV_ADDR            (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - CONFIG_ENV_SIZE - 0x10) /* let space for reset vector */
/* #define CONFIG_ENV_ADDR            (CONFIG_SYS_MONITOR_BASE)*/
#define CONFIG_ENV_OFFSET          (CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#endif

#endif
/* END ENVIRONNEMENT FLASH */
/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CONFIG_SYS_NVRAM_BASE_ADDR	CONFIG_SYS_NVRAM_REG_BASE_ADDR	/* NVRAM base address	*/
#define CONFIG_SYS_NVRAM_SIZE		0x7F8		/* NVRAM size 2kByte - 8 Byte for RTC */

#ifdef CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_SIZE		0x7F8		/* Size of Environment vars	*/
#define CONFIG_ENV_ADDR		\
	(CONFIG_SYS_NVRAM_BASE_ADDR+CONFIG_SYS_NVRAM_SIZE-CONFIG_ENV_SIZE)	/* Env	*/
#endif

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFF800000	/* FLASH bank #0 8MB	*/
#define FLASH_BASE1_PRELIM	0		/* FLASH bank #1	*/


/* Configuration Port location */
/*  #define CONFIG_PORT_ADDR	0xF0000500 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00df0000  /* inside of SDRAM		       */
#define CONFIG_SYS_INIT_RAM_END	0x0f00	/* End of used area in RAM	       */
#define CONFIG_SYS_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS      0x50

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif
#endif	/* __CONFIG_H */
