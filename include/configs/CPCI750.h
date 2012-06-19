/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/*************************************************************************
 * (c) 2004 esd gmbh Hannover
 *
 *
 * from db64360.h file
 * by Reinhard Arlt reinhard.arlt@esd-electronics.com
 *
  ************************************************************************/


#ifndef __CONFIG_H
#define __CONFIG_H

/* This define must be before the core.h include */
#define CONFIG_CPCI750		1	/* this is an CPCI750 board	*/

#ifndef __ASSEMBLY__
#include <../board/Marvell/include/core.h>
#endif
/*-----------------------------------------------------*/

#include "../board/esd/cpci750/local.h"

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_750FX			/* we have a 750FX (override local.h) */

#define CONFIG_CPCI750		1	/* this is an CPCI750 board	*/

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

#define CONFIG_BAUDRATE		9600	/* console baudrate = 9600	*/

#define CONFIG_MV64360_ECC		/* enable ECC support */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* which initialization functions to call for this board */
#define CONFIG_MISC_INIT_R
#define CONFIG_BOARD_PRE_INIT
#define CONFIG_BOARD_EARLY_INIT_F 1

#define CONFIG_SYS_BOARD_NAME		"CPCI750"
#define CONFIG_IDENT_STRING	"Marvell 64360 + IBM750FX"

/*#define CONFIG_SYS_HUSH_PARSER*/
#define CONFIG_SYS_HUSH_PARSER


#define CONFIG_CMDLINE_EDITING		/* add command line history	*/
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support	*/

/* Define which ETH port will be used for connecting the network */
#define CONFIG_SYS_ETH_PORT		ETH_0

/*
 * The following defines let you select what serial you want to use
 * for your console driver.
 *
 * what to do:
 * to use the DUART, undef CONFIG_MPSC.	 If you have hacked a serial
 * cable onto the second DUART channel, change the CONFIG_SYS_DUART port from 1
 * to 0 below.
 *
 * to use the MPSC, #define CONFIG_MPSC.  If you have wired up another
 * mpsc channel, change CONFIG_MPSC_PORT to the desired value.
 */
#define CONFIG_MPSC
#define CONFIG_MPSC_PORT	0

/* to change the default ethernet port, use this define (options: 0, 1, 2) */
#define MV_ETH_DEVS		1
#define CONFIG_ETHER_PORT	0

#undef CONFIG_ETHER_PORT_MII	/* use RMII */

#define CONFIG_BOOTDELAY	5	/* autoboot disabled		*/

#define CONFIG_RTC_M48T35A	1	/* ST Electronics M48 timekeeper */

#define CONFIG_ZERO_BOOTDELAY_CHECK


#undef	CONFIG_BOOTARGS

/* -----------------------------------------------------------------------------
 * New bootcommands for Marvell CPCI750 c 2002 Ingo Assmus
 */

#define CONFIG_IPADDR		"192.168.0.185"

#define CONFIG_SERIAL		"AA000001"
#define CONFIG_SERVERIP		"10.0.0.79"
#define CONFIG_ROOTPATH		"/export/nfs_cpci750/%s"

#define CONFIG_TESTDRAMDATA	y
#define CONFIG_TESTDRAMADDRESS	n
#define CONFIG_TESETDRAMWALK	n

/* ----------------------------------------------------------------------------- */


#define CONFIG_LOADS_ECHO	0	/* echo off for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE		/* allow baudrate changes	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#undef	CONFIG_ALTIVEC			/* undef to disable		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_I2C
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_PCI
#define CONFIG_CMD_ELF
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_IDE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2


#define CONFIG_DOS_PARTITION

#define CONFIG_USE_CPCIDVI

#ifdef	CONFIG_USE_CPCIDVI
#define CONFIG_VIDEO
#define CONFIG_VIDEO_CT69000
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VIDEO_LOGO
#define CONFIG_I8042_KBD
#define CONFIG_SYS_ISA_IO 0
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_SPEED	80000		/* I2C speed default */

#define CONFIG_PRAM 0

#define CONFIG_SYS_GT_DUAL_CPU			/* also for JTAG even with one cpu */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

/*#define CONFIG_SYS_MEMTEST_START	0x00400000*/	/* memtest works on	*/
/*#define CONFIG_SYS_MEMTEST_END		0x00C00000*/	/* 4 ... 12 MB in DRAM	*/
/*#define CONFIG_SYS_MEMTEST_END		0x07c00000*/	/* 4 ... 124 MB in DRAM */

/*
#define CONFIG_SYS_DRAM_TEST
 * DRAM tests
 *   CONFIG_SYS_DRAM_TEST - enables the following tests.
 *
 *   CONFIG_SYS_DRAM_TEST_DATA - Enables test for shorted or open data lines
 *			  Environment variable 'test_dram_data' must be
 *			  set to 'y'.
 *   CONFIG_SYS_DRAM_TEST_DATA - Enables test to verify that each word is uniquely
 *			  addressable. Environment variable
 *			  'test_dram_address' must be set to 'y'.
 *   CONFIG_SYS_DRAM_TEST_WALK - Enables test a 64-bit walking ones pattern test.
 *			  This test takes about 6 minutes to test 64 MB.
 *			  Environment variable 'test_dram_walk' must be
 *			  set to 'y'.
 */
#define CONFIG_SYS_DRAM_TEST
#if defined(CONFIG_SYS_DRAM_TEST)
#define CONFIG_SYS_MEMTEST_START		0x00400000	/* memtest works on	*/
/*#define CONFIG_SYS_MEMTEST_END		0x00C00000*/	/* 4 ... 12 MB in DRAM	*/
#define CONFIG_SYS_MEMTEST_END		0x07c00000	/* 4 ... 124 MB in DRAM */
#define CONFIG_SYS_DRAM_TEST_DATA
#define CONFIG_SYS_DRAM_TEST_ADDRESS
#define CONFIG_SYS_DRAM_TEST_WALK
#endif /* CONFIG_SYS_DRAM_TEST */

#define CONFIG_DISPLAY_MEMMAP		/* at the end of the bootprocess show the memory map */
#undef CONFIG_SYS_DISPLAY_DIMM_SPD_CONTENT	/* show SPD content during boot */

#define CONFIG_SYS_LOAD_ADDR		0x00300000	/* default load address */

#define CONFIG_SYS_HZ			1000		/* decr freq: 1ms ticks */
#define CONFIG_SYS_BUS_CLK		133000000	/* 133 MHz (CPU = 5*Bus = 666MHz)		*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_SYS_TCLK		133000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

 /*
 * When locking data in cache you should point the CONFIG_SYS_INIT_RAM_ADDRESS
 * To an unused memory region. The stack will remain in cache until RAM
 * is initialized
*/
#undef	  CONFIG_SYS_INIT_RAM_LOCK
/* #define CONFIG_SYS_INIT_RAM_ADDR	0x40000000*/ /* unused memory region */
/* #define CONFIG_SYS_INIT_RAM_ADDR	0xfba00000*/ /* unused memory region */
#define CONFIG_SYS_INIT_RAM_ADDR	0xf1080000 /* unused memory region */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

#define RELOCATE_INTERNAL_RAM_ADDR
#ifdef RELOCATE_INTERNAL_RAM_ADDR
/*#define CONFIG_SYS_INTERNAL_RAM_ADDR 0xfba00000*/
#define CONFIG_SYS_INTERNAL_RAM_ADDR	0xf1080000
#endif

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
/* Dummies for BAT 4-7 */
#define CONFIG_SYS_SDRAM1_BASE		0x10000000	/* each 256 MByte */
#define CONFIG_SYS_SDRAM2_BASE		0x20000000
#define CONFIG_SYS_SDRAM3_BASE		0x30000000
#define CONFIG_SYS_SDRAM4_BASE		0x40000000
#define CONFIG_SYS_RESET_ADDRESS	0xfff00100
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MONITOR_BASE	0xfff00000
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 256 kB for malloc */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI		1	   /* Flash is CFI conformant		*/
#define CONFIG_SYS_FLASH_PROTECTION	1	   /* use hardware protection		*/
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	   /* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_BASE		0xfc000000 /* start of flash banks		*/
#define CONFIG_SYS_MAX_FLASH_BANKS	4	   /* max number of memory banks	*/
#define CONFIG_SYS_FLASH_INCREMENT	0x01000000 /* size of  flash bank		*/
#define CONFIG_SYS_MAX_FLASH_SECT	128	   /* max number of sectors on one chip */
#define CONFIG_SYS_FLASH_BANKS_LIST  { CONFIG_SYS_FLASH_BASE,				   \
				CONFIG_SYS_FLASH_BASE + 1*CONFIG_SYS_FLASH_INCREMENT,	   \
				CONFIG_SYS_FLASH_BASE + 2*CONFIG_SYS_FLASH_INCREMENT,	   \
				CONFIG_SYS_FLASH_BASE + 3*CONFIG_SYS_FLASH_INCREMENT }
#define CONFIG_SYS_FLASH_EMPTY_INFO	1	   /* show if bank is empty		*/

/* areas to map different things with the GT in physical space */
#define CONFIG_SYS_DRAM_BANKS		4

/* What to put in the bats. */
#define CONFIG_SYS_MISC_REGION_BASE	0xf0000000

/* Peripheral Device section */

/*******************************************************/
/* We have on the cpci750 Board :		       */
/* GT-Chipset Register Area			       */
/* GT-Chipset internal SRAM 256k		       */
/* SRAM on external device module		       */
/* Real time clock on external device module	       */
/* dobble UART on external device module	       */
/* Data flash on external device module		       */
/* Boot flash on external device module		       */
/*******************************************************/
#define CONFIG_SYS_DFL_GT_REGS		0x14000000				/* boot time GT_REGS */
#define	 CONFIG_SYS_CPCI750_RESET_ADDR 0x14000000				/* After power on Reset the CPCI750 is here */

#undef	MARVEL_STANDARD_CFG
#ifndef		MARVEL_STANDARD_CFG
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define CONFIG_SYS_GT_REGS		0xf1000000				/* GT Registers will be mapped here */
/*#define CONFIG_SYS_DEV_BASE		0xfc000000*/				/* GT Devices CS start here */
#define CONFIG_SYS_INT_SRAM_BASE	0xf1080000				/* GT offers 256k internal fast SRAM */

#define CONFIG_SYS_BOOT_SPACE		0xff000000				/* BOOT_CS0 flash 0    */
#define CONFIG_SYS_DEV0_SPACE		0xfc000000				/* DEV_CS0 flash 1     */
#define CONFIG_SYS_DEV1_SPACE		0xfd000000				/* DEV_CS1 flash 2     */
#define CONFIG_SYS_DEV2_SPACE		0xfe000000				/* DEV_CS2 flash 3     */
#define CONFIG_SYS_DEV3_SPACE		0xf0000000				/* DEV_CS3 nvram/can   */

#define CONFIG_SYS_BOOT_SIZE		_16M					/* cpci750 flash 0     */
#define CONFIG_SYS_DEV0_SIZE		_16M					/* cpci750 flash 1     */
#define CONFIG_SYS_DEV1_SIZE		_16M					/* cpci750 flash 2     */
#define CONFIG_SYS_DEV2_SIZE		_16M					/* cpci750 flash 3     */
#define CONFIG_SYS_DEV3_SIZE		_16M					/* cpci750 nvram/can   */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#endif

/* Reset values for Port behavior (8bit/ 32bit, etc.) only corrected by device width */
#define CONFIG_SYS_DEV0_PAR		0x8FDFFFFF				/* 16 bit flash */
#define CONFIG_SYS_DEV1_PAR		0x8FDFFFFF				/* 16 bit flash */
#define CONFIG_SYS_DEV2_PAR		0x8FDFFFFF				/* 16 bit flash */
#define CONFIG_SYS_DEV3_PAR		0x8FCFFFFF				/* nvram/can	*/
#define CONFIG_SYS_BOOT_PAR		0x8FDFFFFF				/* 16 bit flash */

	/*   c	  4    a      8	    2	  4    1      c		*/
	/* 33 22|2222|22 22|111 1|11 11|1 1  |	  |		*/
	/* 10 98|7654|32 10|987 6|54 32|1 098|7 654|3 210	*/
	/* 11|00|0100|10 10|100|0 00|10 0|100 0|001 1|100	*/
	/*  3| 0|.... ..| 2| 4 |  0 |  4 |  8  |  3  | 4	*/


/* MPP Control MV64360 Appendix P P. 632*/
#define CONFIG_SYS_MPP_CONTROL_0	0x00002222	/*				     */
#define CONFIG_SYS_MPP_CONTROL_1	0x11110000	/*				     */
#define CONFIG_SYS_MPP_CONTROL_2	0x11111111	/*				     */
#define CONFIG_SYS_MPP_CONTROL_3	0x00001111	/*				     */
/* #define CONFIG_SYS_SERIAL_PORT_MUX	0x00000102*/	/*				     */


#define CONFIG_SYS_GPP_LEVEL_CONTROL	0xffffffff	/* 1111 1111 1111 1111 1111 1111 1111 1111*/

/* setup new config_value for MV64360 DDR-RAM To_do !! */
/*# define CONFIG_SYS_SDRAM_CONFIG	0xd8e18200*/	/* 0x448 */
/*# define CONFIG_SYS_SDRAM_CONFIG	0xd8e14400*/	/* 0x1400 */
				/* GB has high prio.
				   idma has low prio
				   MPSC has low prio
				   pci has low prio 1 and 2
				   cpu has high prio
				   Data DQS pins == eight (DQS[7:0] foe x8 and x16 devices
				   ECC disable
				   non registered DRAM */
				/* 31:26   25:22  21:20 19 18 17 16 */
				/* 100001 0000	 010   0   0   0  0 */
				/* refresh_count=0x400
				   phisical interleaving disable
				   virtual interleaving enable */
				/* 15 14 13:0 */
				/* 0  1	 0x400 */
# define CONFIG_SYS_SDRAM_CONFIG	0x58200400	/* 0x1400  copied from Dink32 bzw. VxWorks*/


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */

#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW		/* show devices on bus		*/

/* PCI MEMORY MAP section */
#define CONFIG_SYS_PCI0_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI0_MEM_SIZE	_128M
#define CONFIG_SYS_PCI1_MEM_BASE	0x88000000
#define CONFIG_SYS_PCI1_MEM_SIZE	_128M

#define CONFIG_SYS_PCI0_0_MEM_SPACE	(CONFIG_SYS_PCI0_MEM_BASE)
#define CONFIG_SYS_PCI1_0_MEM_SPACE	(CONFIG_SYS_PCI1_MEM_BASE)

/* PCI I/O MAP section */
#define CONFIG_SYS_PCI0_IO_BASE	0xfa000000
#define CONFIG_SYS_PCI0_IO_SIZE	_16M
#define CONFIG_SYS_PCI1_IO_BASE	0xfb000000
#define CONFIG_SYS_PCI1_IO_SIZE	_16M

#define CONFIG_SYS_PCI0_IO_SPACE	(CONFIG_SYS_PCI0_IO_BASE)
#define CONFIG_SYS_PCI0_IO_SPACE_PCI	0x00000000
#define CONFIG_SYS_PCI1_IO_SPACE	(CONFIG_SYS_PCI1_IO_BASE)
#define CONFIG_SYS_PCI1_IO_SPACE_PCI	0x00000000

#define CONFIG_SYS_ISA_IO_BASE_ADDRESS (CONFIG_SYS_PCI0_IO_BASE)

#if defined (CONFIG_750CX)
#define CONFIG_SYS_PCI_IDSEL 0x0
#else
#define CONFIG_SYS_PCI_IDSEL 0x30
#endif

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#undef	CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#define CONFIG_IDE_RESET		/* no reset for ide supported	*/
#define CONFIG_IDE_PREINIT		/* check for units		*/

#define CONFIG_SYS_IDE_MAXBUS		2		/* max. 1 IDE busses	*/
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*2) /* max. 1 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR	0
#define CONFIG_SYS_ATA_IDE0_OFFSET	0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_REG_OFFSET	0x0000	/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0000	/* Offset for alternate registers	*/
#ifndef __ASSEMBLY__
int ata_device(int dev);
#endif
#define ATA_DEVICE(dev)                 ata_device(dev)
#define CONFIG_ATAPI                    1

/*----------------------------------------------------------------------
 * Initial BAT mappings
 */

/* NOTES:
 * 1) GUARDED and WRITE_THRU not allowed in IBATS
 * 2) CACHEINHIBIT and WRITETHROUGH not allowed together in same BAT
 */

/* SDRAM */
#define CONFIG_SYS_IBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT0U (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT0U CONFIG_SYS_IBAT0U

/* init ram */
#define CONFIG_SYS_IBAT1L  (CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U  (CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_256K | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L  CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U  CONFIG_SYS_IBAT1U

/* PCI0, PCI1 in one BAT */
#define CONFIG_SYS_IBAT2L BATL_NO_ACCESS
#define CONFIG_SYS_IBAT2U CONFIG_SYS_DBAT2U
#define CONFIG_SYS_DBAT2L (CONFIG_SYS_PCI0_MEM_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U (CONFIG_SYS_PCI0_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* GT regs, bootrom, all the devices, PCI I/O */
#define CONFIG_SYS_IBAT3L (CONFIG_SYS_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW)
#define CONFIG_SYS_IBAT3U (CONFIG_SYS_MISC_REGION_BASE | BATU_VS | BATU_VP | BATU_BL_256M)
#define CONFIG_SYS_DBAT3L (CONFIG_SYS_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U CONFIG_SYS_IBAT3U

/*
 * 750FX IBAT and DBAT pairs (To_do: define regins for I(D)BAT4 - I(D)BAT7)
 * IBAT4 and DBAT4
 * FIXME: ingo disable BATs for Linux Kernel
 */
/* #undef SETUP_HIGH_BATS_FX750	*/	/* don't initialize BATS 4-7 */
#define SETUP_HIGH_BATS_FX750		/* initialize BATS 4-7 */

#ifdef SETUP_HIGH_BATS_FX750
#define CONFIG_SYS_IBAT4L (CONFIG_SYS_SDRAM1_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT4U (CONFIG_SYS_SDRAM1_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT4L (CONFIG_SYS_SDRAM1_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U CONFIG_SYS_IBAT4U

/* IBAT5 and DBAT5 */
#define CONFIG_SYS_IBAT5L (CONFIG_SYS_SDRAM2_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT5U (CONFIG_SYS_SDRAM2_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L (CONFIG_SYS_SDRAM2_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U CONFIG_SYS_IBAT5U

/* IBAT6 and DBAT6 */
#define CONFIG_SYS_IBAT6L (CONFIG_SYS_SDRAM3_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT6U (CONFIG_SYS_SDRAM3_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L (CONFIG_SYS_SDRAM3_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U CONFIG_SYS_IBAT6U

/* IBAT7 and DBAT7 */
#define CONFIG_SYS_IBAT7L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT7U (CONFIG_SYS_SDRAM4_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT7L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT7U CONFIG_SYS_IBAT7U

#else		/* set em out of range for Linux !!!!!!!!!!! */
#define CONFIG_SYS_IBAT4L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT4U (CONFIG_SYS_SDRAM4_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT4L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U CONFIG_SYS_IBAT4U

/* IBAT5 and DBAT5 */
#define CONFIG_SYS_IBAT5L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT5U (CONFIG_SYS_SDRAM4_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U CONFIG_SYS_IBAT4U

/* IBAT6 and DBAT6 */
#define CONFIG_SYS_IBAT6L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT6U (CONFIG_SYS_SDRAM4_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U CONFIG_SYS_IBAT4U

/* IBAT7 and DBAT7 */
#define CONFIG_SYS_IBAT7L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT7U (CONFIG_SYS_SDRAM4_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT7L (CONFIG_SYS_SDRAM4_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT7U CONFIG_SYS_IBAT4U

#endif
/* FIXME: ingo end: disable BATs for Linux Kernel */

/* I2C addresses for the two DIMM SPD chips */
#define DIMM0_I2C_ADDR	0x51
#define DIMM1_I2C_ADDR	0x52

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8<<20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_BOOT_FLASH_WIDTH	2	/* 16 bit */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT	500	/* Timeout for Flash Lock (in ms) */

#if 0
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_ADDR		0xFFF78000 /* Marvell 8-Bit Bootflash last sector */
/* #define CONFIG_ENV_ADDR	   (CONFIG_SYS_FLASH_BASE+CONFIG_SYS_MONITOR_LEN-CONFIG_ENV_SECT_SIZE) */
#endif

#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 20
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x050
#define CONFIG_ENV_OFFSET		0x200	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x600	/* 2048 bytes may be used for env vars*/

#define CONFIG_SYS_NVRAM_BASE_ADDR	0xf0000000		/* NVRAM base address	*/
#define CONFIG_SYS_NVRAM_SIZE		(32*1024)		/* NVRAM size		*/
#define CONFIG_SYS_VXWORKS_MAC_PTR	(CONFIG_SYS_NVRAM_BASE_ADDR+CONFIG_SYS_NVRAM_SIZE-0x40)

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For all MPC74xx CPUs		 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/mpc74xx.h for the defines used here
 */

/*#define CONFIG_SYS_L2*/
#undef CONFIG_SYS_L2

/*    #ifdef CONFIG_750CX*/
#if defined (CONFIG_750CX) || defined (CONFIG_750FX)
#define L2_INIT 0
#else
#define L2_INIT		(L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
			L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
#endif

#define L2_ENABLE	(L2_INIT | L2CR_L2E)

#define CONFIG_SYS_BOARD_ASM_INIT	1

#define CPCI750_SLAVE_TEST	(((in8(0xf0300000) & 0x80) == 0) ? 0 : 1)
#define CPCI750_ECC_TEST	(((in8(0xf0300000) & 0x02) == 0) ? 1 : 0)
#define CONFIG_SYS_PLD_VER	0xf0e00000

#define CONFIG_OF_LIBFDT 1

#endif	/* __CONFIG_H */
