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

#ifndef __CONFIG_H
#define __CONFIG_H

/* This define must be before the core.h include */
#define CONFIG_DB64460		1	/* this is an DB64460 board	*/

#ifndef __ASSEMBLY__
#include "../board/Marvell/include/core.h"
#endif

/*-----------------------------------------------------*/
/*    #include "../board/db64460/local.h"	      */
#ifndef __LOCAL_H
#define __LOCAL_H

#define CONFIG_ETHADDR		64:46:00:00:00:01
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		64:46:00:00:00:02
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR		64:46:00:00:00:03

#define CONFIG_ENV_OVERWRITE
#endif	/* __CONFIG_H */

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_74xx			/* we have a 750FX (override local.h) */

#define CONFIG_DB64460		1	/* this is an DB64460 board	*/

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115000	*/
/*ronen - we don't use the global CONFIG_ECC, since in the global ecc we initialize the
	DRAM for ECC in the phase we are relocating to it, which isn't so sufficient.
	so we will define our ECC CONFIG and initilize the DRAM for ECC in the DRAM initialization phase,
	see sdram_init.c   */
#undef CONFIG_ECC	 /* enable ECC support */
#define CONFIG_MV64460_ECC

/* which initialization functions to call for this board */
#define CONFIG_MISC_INIT_R     /* initialize the icache L1 */
#define CONFIG_BOARD_EARLY_INIT_F

#define CFG_BOARD_NAME		"DB64460"
#define CONFIG_IDENT_STRING	"Marvell DB64460 (1.0)"

/*#define CFG_HUSH_PARSER */
#undef CFG_HUSH_PARSER

#define CFG_PROMPT_HUSH_PS2	"> "

/*
 * The following defines let you select what serial you want to use
 * for your console driver.
 *
 * what to do:
 * to use the DUART, undef CONFIG_MPSC.	 If you have hacked a serial
 * cable onto the second DUART channel, change the CFG_DUART port from 1
 * to 0 below.
 *
 * to use the MPSC, #define CONFIG_MPSC.  If you have wired up another
 * mpsc channel, change CONFIG_MPSC_PORT to the desired value.
 */

#define CONFIG_MPSC_PORT	0

/* to change the default ethernet port, use this define (options: 0, 1, 2) */
#define CONFIG_NET_MULTI
#define MV_ETH_DEVS 3

/* #undef CONFIG_ETHER_PORT_MII	 */
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_ZERO_BOOTDELAY_CHECK


#undef	CONFIG_BOOTARGS
/*#define CONFIG_PREBOOT	"echo;echo Type \"run flash_nfs\" to mount root filesystem over NFS;echo" */

/* ronen - autoboot using tftp */
#if (CONFIG_BOOTDELAY >= 0)
#define CONFIG_BOOTCOMMAND	"tftpboot 0x400000 uImage;\
 setenv bootargs ${bootargs} ${bootargs_root} nfsroot=${serverip}:${rootpath} \
 ip=${ipaddr}:${serverip}${bootargs_end};  bootm 0x400000; "

#define CONFIG_BOOTARGS "console=ttyS0,115200"

#endif

/* ronen - the u-boot.bin should be ~0x30000 bytes */
#define CONFIG_EXTRA_ENV_SETTINGS \
     "burn_uboot_sep= tftp 100000 u-boot.bin;protect off all;era FFF00000 FFF4ffff; \
cp.b 100000 FFF00000 0x40000;protect on 1:0-4;\0" \
      "burn_uboot_dep= tftp 100000 u-boot.bin;protect off all;era FFF00000 FFF7ffff; \
cp.b 100000 FFF00000 0x40000;protect on 1:0-7;\0" \
      "bootargs_root=root=/dev/nfs rw\0" \
      "bootargs_end=:::DB64460:eth0:none \0"\
      "ethprime=mv_enet0\0"\
      "standalone=fsload 0x400000 uImage;setenv bootargs ${bootargs} root=/dev/mtdblock/0 rw \
ip=${ipaddr}:${serverip}${bootargs_end}; bootm 0x400000;\0"

/* --------------------------------------------------------------------------------------------------------------- */
/* New bootcommands for Marvell DB64460 c 2002 Ingo Assmus */

#define CONFIG_IPADDR		10.2.40.90

#define CONFIG_SERIAL		"No. 1"
#define CONFIG_SERVERIP		10.2.1.126
#define CONFIG_ROOTPATH /mnt/yellow_dog_mini


#define CONFIG_TESTDRAMDATA	y
#define CONFIG_TESTDRAMADDRESS	n
#define CONFIG_TESETDRAMWALK	n

/* --------------------------------------------------------------------------------------------------------------- */

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download */
#define CFG_LOADS_BAUD_CHANGE		/* allow baudrate changes	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#undef	CONFIG_ALTIVEC			/* undef to disable		*/

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | \
				 CONFIG_BOOTP_BOOTFILESIZE)
/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor1"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */

/* Use first bank for JFFS2, second bank contains U-Boot.
 *
 * Note: fake mtd_id's used, no linux mtd map file.
 */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor1=db64460-1"
#define MTDPARTS_DEFAULT	"mtdparts=db64460-1:-(jffs2)"
*/

#define CONFIG_COMMANDS (CONFIG_CMD_DFL \
			 | CFG_CMD_ASKENV \
			 | CFG_CMD_I2C \
			 | CFG_CMD_EEPROM \
			 | CFG_CMD_CACHE \
			 | CFG_CMD_JFFS2 \
			 | CFG_CMD_PCI \
			 | CFG_CMD_NET )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_SPEED	40000		/* I2C speed default */

/* #define CFG_GT_DUAL_CPU	 also for JTAG even with one cpu */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

/*#define CFG_MEMTEST_START	0x00400000	 memtest works on	*/
/*#define CFG_MEMTEST_END		0x00C00000	 4 ... 12 MB in DRAM	*/
/*#define CFG_MEMTEST_END		0x07c00000	 4 ... 124 MB in DRAM	*/

/*
#define CFG_DRAM_TEST
 * DRAM tests
 *   CFG_DRAM_TEST - enables the following tests.
 *
 *   CFG_DRAM_TEST_DATA - Enables test for shorted or open data lines
 *			  Environment variable 'test_dram_data' must be
 *			  set to 'y'.
 *   CFG_DRAM_TEST_DATA - Enables test to verify that each word is uniquely
 *			  addressable. Environment variable
 *			  'test_dram_address' must be set to 'y'.
 *   CFG_DRAM_TEST_WALK - Enables test a 64-bit walking ones pattern test.
 *			  This test takes about 6 minutes to test 64 MB.
 *			  Environment variable 'test_dram_walk' must be
 *			  set to 'y'.
 */
#define CFG_DRAM_TEST
#if defined(CFG_DRAM_TEST)
#define CFG_MEMTEST_START		0x00400000	/* memtest works on	*/
/* #define CFG_MEMTEST_END		0x00C00000	 4 ... 12 MB in DRAM	*/
#define CFG_MEMTEST_END		0x07c00000	/* 4 ... 124 MB in DRAM */
#define CFG_DRAM_TEST_DATA
#define CFG_DRAM_TEST_ADDRESS
#define CFG_DRAM_TEST_WALK
#endif /* CFG_DRAM_TEST */

#undef CONFIG_DISPLAY_MEMMAP		/* at the end of the bootprocess show the memory map */
#undef CFG_DISPLAY_DIMM_SPD_CONTENT	/* show SPD content during boot */

#define CFG_LOAD_ADDR		0x00400000	/* default load address */

#define CFG_HZ			1000		/* decr freq: 1ms ticks */
/*ronen - this the Sys clock (cpu bus,internal dram and SDRAM) */
#define CFG_BUS_HZ		133000000	/* 133 MHz (CPU = 5*Bus = 666MHz)		*/
#define CFG_BUS_CLK		CFG_BUS_HZ

#define CFG_DDR_SDRAM_CYCLE_COUNT_LOP 7 /* define the SDRAM cycle count */
#define CFG_DDR_SDRAM_CYCLE_COUNT_ROP 50 /* for 200MHZ -> 5.0 ns, 166MHZ -> 6.0, 133MHZ -> 7.50 ns */

/*ronen - this is the Tclk (MV64460 core) */
#define CFG_TCLK		133000000


#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CFG_750FX_HID0		0x8000c084
#define CFG_750FX_HID1		0x54800000
#define CFG_750FX_HID2		0x00000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

/*
 * When locking data in cache you should point the CFG_INIT_RAM_ADDRESS
 * To an unused memory region. The stack will remain in cache until RAM
 * is initialized
*/
#define CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0x40000000 /* unused memory region */
#define CFG_INIT_RAM_END	0x1000
#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for init data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

#define RELOCATE_INTERNAL_RAM_ADDR
#ifdef RELOCATE_INTERNAL_RAM_ADDR
	#define CFG_INTERNAL_RAM_ADDR	0xf8000000
#endif

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
/* Dummies for BAT 4-7 */
#define CFG_SDRAM1_BASE		0x10000000	/* each 256 MByte */
#define CFG_SDRAM2_BASE		0x20000000
#define CFG_SDRAM3_BASE		0x30000000
#define CFG_SDRAM4_BASE		0x40000000
#define CFG_FLASH_BASE			0xfff00000

#define CFG_DFL_BOOTCS_BASE	0xff800000
#define CONFIG_VERY_BIG_RAM		/* we will use up to 256M memory for cause we are short of BATS*/

#define BRIDGE_REG_BASE_BOOTM 0xfbe00000 /* this paramaters are used when booting the linux kernel */
#define UART_BASE_BOOTM	      0xfbb00000 /* in order to be sync with the kernel parameters. */
#define PCI0_IO_BASE_BOOTM    0xfd000000

#define CFG_RESET_ADDRESS		0xfff00100
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE		CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */

/* areas to map different things with the GT in physical space */
#define CFG_DRAM_BANKS		4

/* What to put in the bats. */
#define CFG_MISC_REGION_BASE	0xf0000000

/* Peripheral Device section */

/*******************************************************/
/* We have on the db64460 Board :		   */
/* GT-Chipset Register Area				*/
/* GT-Chipset internal SRAM 256k		    */
/* SRAM on external device module		   */
/* Real time clock on external device module	  */
/* dobble UART on external device module	 */
/* Data flash on external device module		    */
/* Boot flash on external device module		    */
/*******************************************************/
#define CFG_DFL_GT_REGS		0x14000000				/* boot time GT_REGS */
#define	 CFG_DB64460_RESET_ADDR 0x14000000				/* After power on Reset the DB64460 is here */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define CFG_GT_REGS		0xf1000000				/* GT Registers will be mapped here */
#define CFG_DEV_BASE		0xfc000000				/* GT Devices CS start here */

#define CFG_DEV0_SPACE		CFG_DEV_BASE				/* DEV_CS0 device modul sram */
#define CFG_DEV1_SPACE		(CFG_DEV0_SPACE + CFG_DEV0_SIZE)	/* DEV_CS1 device modul real time clock (rtc) */
#define CFG_DEV2_SPACE		(CFG_DEV1_SPACE + CFG_DEV1_SIZE)	/* DEV_CS2 device modul doubel uart (duart) */
#define CFG_DEV3_SPACE		(CFG_DEV2_SPACE + CFG_DEV2_SIZE)	/* DEV_CS3 device modul large flash */

#define CFG_DEV0_SIZE		 _8M					/* db64460 sram	 @ 0xfc00.0000 */
#define CFG_DEV1_SIZE		 _8M					/* db64460 rtc	 @ 0xfc80.0000 */
#define CFG_DEV2_SIZE		_16M					/* db64460 duart @ 0xfd00.0000 */
#define CFG_DEV3_SIZE		_16M					/* db64460 flash @ 0xfe00.0000 */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Reset values for Port behavior (8bit/ 32bit, etc.) only corrected by device width */
#define CFG_DEV0_PAR		0x8FEFFFFF				/* 32Bit  sram */
#define CFG_DEV1_PAR		0x8FCFFFFF				/* 8Bit	 rtc */
#define CFG_DEV2_PAR		0x8FCFFFFF				/* 8Bit duart */
#define CFG_8BIT_BOOT_PAR	0x8FCFFFFF				/* 8Bit flash */
#define CFG_32BIT_BOOT_PAR	0x8FEFFFFF				/* 32Bit flash */

	/*   c	  4    a      8	    2	  4    1      c		*/
	/* 33 22|2222|22 22|111 1|11 11|1 1  |	  |		*/
	/* 10 98|7654|32 10|987 6|54 32|1 098|7 654|3 210	*/
	/* 11|00|0100|10 10|100|0 00|10 0|100 0|001 1|100	*/
	/*  3| 0|.... ..| 2| 4 |  0 |  4 |  8  |  3  | 4	*/


/* ronen - update MPP Control MV64460*/
#define CFG_MPP_CONTROL_0	0x02222222
#define CFG_MPP_CONTROL_1	0x11333011
#define CFG_MPP_CONTROL_2	0x40431111
#define CFG_MPP_CONTROL_3	0x00000044

/*# define CFG_SERIAL_PORT_MUX	0x00000102	 0=hiZ	1=MPSC0 2=ETH 0 and 2 RMII */


# define CFG_GPP_LEVEL_CONTROL	0x2c600000	/* 1111 1001 0000 1111 1100 0000 0000 0000*/
							/* gpp[31]		gpp[30]		gpp[29]		gpp[28] */
				/* gpp[27]			gpp[24]*/
							/* gpp[19:14] */

/* setup new config_value for MV64460 DDR-RAM !! */
# define CFG_SDRAM_CONFIG	0x58200400	/* 0x1400  copied from Dink32 bzw. VxWorks*/

#define CFG_DUART_IO		CFG_DEV2_SPACE
#define CFG_DUART_CHAN		1		/* channel to use for console */
#define CFG_INIT_CHAN1
#define CFG_INIT_CHAN2

#define SRAM_BASE		CFG_DEV0_SPACE
#define SRAM_SIZE		0x00100000		/* 1 MB of sram */


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
#define CONFIG_EEPRO100			/* ronen - Support for Intel 82557/82559/82559ER chips */

/* PCI MEMORY MAP section */
#define CFG_PCI0_MEM_BASE	0x80000000
#define CFG_PCI0_MEM_SIZE	_128M
#define CFG_PCI1_MEM_BASE	0x88000000
#define CFG_PCI1_MEM_SIZE	_128M

#define CFG_PCI0_0_MEM_SPACE	(CFG_PCI0_MEM_BASE)
#define CFG_PCI1_0_MEM_SPACE	(CFG_PCI1_MEM_BASE)

/* PCI I/O MAP section */
#define CFG_PCI0_IO_BASE	0xfa000000
#define CFG_PCI0_IO_SIZE	_16M
#define CFG_PCI1_IO_BASE	0xfb000000
#define CFG_PCI1_IO_SIZE	_16M

#define CFG_PCI0_IO_SPACE	(CFG_PCI0_IO_BASE)
#define CFG_PCI0_IO_SPACE_PCI	(CFG_PCI0_IO_BASE) /* ronen we want phy=bus 0x00000000 */
#define CFG_PCI1_IO_SPACE	(CFG_PCI1_IO_BASE)
#define CFG_PCI1_IO_SPACE_PCI	(CFG_PCI1_IO_BASE) /* ronen we want phy=bus 0x00000000 */

#if defined (CONFIG_750CX)
#define CFG_PCI_IDSEL 0x0
#else
#define CFG_PCI_IDSEL 0x30
#endif
/*----------------------------------------------------------------------
 * Initial BAT mappings
 */

/* NOTES:
 * 1) GUARDED and WRITE_THRU not allowed in IBATS
 * 2) CACHEINHIBIT and WRITETHROUGH not allowed together in same BAT
 */

/* SDRAM */
#define CFG_IBAT0L (CFG_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT0U (CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT0L (CFG_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT0U CFG_IBAT0U

/* init ram */
#define CFG_IBAT1L  (CFG_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_IBAT1U  (CFG_INIT_RAM_ADDR | BATU_BL_256K | BATU_VS | BATU_VP)
#define CFG_DBAT1L  CFG_IBAT1L
#define CFG_DBAT1U  CFG_IBAT1U

/* PCI0, PCI1 in one BAT */
#define CFG_IBAT2L BATL_NO_ACCESS
#define CFG_IBAT2U CFG_DBAT2U
#define CFG_DBAT2L (CFG_PCI0_MEM_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CFG_DBAT2U (CFG_PCI0_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* GT regs, bootrom, all the devices, PCI I/O */
#define CFG_IBAT3L (CFG_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW)
#define CFG_IBAT3U (CFG_MISC_REGION_BASE | BATU_VS | BATU_VP | BATU_BL_256M)
#define CFG_DBAT3L (CFG_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U CFG_IBAT3U

/* I2C addresses for the two DIMM SPD chips */
#define DIMM0_I2C_ADDR	0x56
#define DIMM1_I2C_ADDR	0x54

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8<<20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks	*/
#define CFG_MAX_FLASH_SECT	67	/* max number of sectors on one chip */

#define CFG_EXTRA_FLASH_DEVICE	DEVICE3 /* extra flash at device 3 */
#define CFG_EXTRA_FLASH_WIDTH	4	/* 32 bit */
#define CFG_BOOT_FLASH_WIDTH	1	/* 8 bit */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */
#define CFG_FLASH_LOCK_TOUT	500	/* Timeout for Flash Lock (in ms) */
#define CFG_FLASH_CFI		1

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CFG_ENV_SECT_SIZE	0x10000
#define CFG_ENV_ADDR	      0xFFF78000 /* Marvell 8-Bit Bootflash last sector */
/* #define CFG_ENV_ADDR	   (CFG_FLASH_BASE+CFG_MONITOR_LEN-CFG_ENV_SECT_SIZE) */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For all MPC74xx CPUs		 */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/mpc74xx.h for the defines used here
 */

#define CFG_L2


#if defined (CONFIG_750CX) || defined (CONFIG_750FX)
#define L2_INIT 0
#else

#define L2_INIT		0
/*
#define L2_INIT		(L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
			L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
*/
#endif

#define L2_ENABLE	(L2_INIT | L2CR_L2E)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot		    */

#define CFG_BOARD_ASM_INIT	1

#endif	/* __CONFIG_H */
