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

#define CFG_GT_6426x        GT_64260 /* with a 64260 system controller */
#define CONFIG_ETHER_PORT_MII	/* use two MII ports */
#define CONFIG_INTEL_LXT97X	/* Intel LXT97X phy */

#ifndef __ASSEMBLY__
#include <galileo/core.h>
#endif

#include "../board/evb64260/local.h"

#define CONFIG_EVB64260		1	/* this is an EVB64260 board	*/
#define CONFIG_ZUMA_V2		1	/* always define this for ZUMA v2 */

/* #define CONFIG_ZUMA_V2_OLD	1 */	/* backwards compat for old V2 board */

#define CONFIG_BAUDRATE		38400	/* console baudrate = 38400	*/

#define CONFIG_ECC			/* enable ECC support */

#define CONFIG_750CX			/* we have a 750CX/CXe (override local.h) */

/* which initialization functions to call for this board */
#define CONFIG_MISC_INIT_R
#define CONFIG_BOARD_EARLY_INIT_F
#define CFG_BOARD_ASM_INIT

#define CFG_BOARD_NAME		"Zuma APv2"

#define CFG_HUSH_PARSER
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
#define CONFIG_MPSC

#define CONFIG_MPSC_PORT	0

#define CONFIG_NET_MULTI        /* attempt all available adapters */

/* define this if you want to enable GT MAC filtering */
#define CONFIG_GT_USE_MAC_HASH_TABLE

#if 1
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_ZERO_BOOTDELAY_CHECK

#undef	CONFIG_BOOTARGS

#define CONFIG_BOOTCOMMAND							\
	"tftpboot && "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath " \
	"ip=$ipaddr:$serverip:$gatewayip:"				\
	"$netmask:$hostname:eth0:none panic=5 && bootm"

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download */
#define CFG_LOADS_BAUD_CHANGE		/* allow baudrate changes	*/

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

#define CONFIG_MII		/* enable MII commands */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BSP
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE


/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor1=zuma-1,nor2=zuma-2"
#define MTDPARTS_DEFAULT	"mtdparts=zuma-1:-(jffs2),zuma-2:-(user)"
*/

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

#define CFG_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x00300000	/* default load address */

#define CFG_HZ			1000		/* decr freq: 1ms ticks */

#define CFG_BUS_HZ		133000000	/* 133 MHz		*/

#define CFG_BUS_CLK		CFG_BUS_HZ

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CFG_INIT_RAM_ADDR	0x40000000
#define CFG_INIT_RAM_END	0x1000
#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for init data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_RAM_LOCK


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xfff00000
#define CFG_RESET_ADDRESS	0xfff00100
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */

/* areas to map different things with the GT in physical space */
#define CFG_DRAM_BANKS		4
#define CFG_DFL_GT_REGS		0x14000000	/* boot time GT_REGS */

/* What to put in the bats. */
#define CFG_MISC_REGION_BASE	0xf0000000

/* Peripheral Device section */
#define CFG_GT_REGS		0xf8000000	/* later mapped GT_REGS */
#define CFG_DEV_BASE		0xf0000000
#define CFG_DEV0_SIZE		_64M /* zuma flash @ 0xf000.0000*/
#define CFG_DEV1_SIZE		 _8M /* zuma IDE   @ 0xf400.0000 */
#define CFG_DEV2_SIZE		 _8M /* unused */
#define CFG_DEV3_SIZE		 _8M /* unused */

#define CFG_DEV0_PAR		0xc498243c
	/*     c    4     9     8     2	    4     3     c */
	/* 33 22|2222|22 22|111 1|11 11|1 1  |	   |      */
	/* 10 98|7654|32 10|987 6|54 32|1 098|7 654|3 210 */
	/* 11|00|0100|10 01|100|0 00|10 0|100 0|011 1|100 */
	/*  3| 0|.... ..| 1| 4 |  0 |  4 |   8 |   7 | 4  */

#define CFG_DEV1_PAR		0xc01b6ac5
	/*     c    0     1     b     6	    a     c     5 */
	/* 33 22|2222|22 22|111 1|11 11|1 1  |	   |      */
	/* 10 98|7654|32 10|987 6|54 32|1 098|7 654|3 210 */
	/* 11|00|0000|00 01|101|1 01|10 1|010 1|100 0|101 */
	/*  3| 0|.... ..| 1| 5 |  5 |  5 |   5 |   8 | 5  */


#define CFG_8BIT_BOOT_PAR	0xc00b5e7c

#define CFG_MPP_CONTROL_0	0x00007777 /* GPP[7:4] : REQ0[1:0] GNT0[1:0] */
#define CFG_MPP_CONTROL_1	0x00000000 /* GPP[15:12] : GPP[11:8] */
#define CFG_MPP_CONTROL_2	0x00008888 /* GPP[23:20] : REQ1[1:0] GNT1[1:0] */
#define CFG_MPP_CONTROL_3	0x00000000 /* GPP[31:28] (int[3:0]) */
					   /* GPP[27:24] (27 is int4, rest are GPP) */

#define CFG_SERIAL_PORT_MUX	0x00001101 /* 11=MPSC1/MPSC0 01=ETH,  0=only MII */
#define CFG_GPP_LEVEL_CONTROL	0xf8000000 /* interrupt inputs: GPP[31:27] */

#define CFG_SDRAM_CONFIG	0xe4e18200	/* 0x448 */
				/* idmas use buffer 1,1
				   comm use buffer 1
				   pci use buffer 0,0 (pci1->0 pci0->0)
				   cpu use buffer 1 (R*18)
				   normal load (see also ifdef HVL)
				   standard SDRAM (see also ifdef REG)
				   non staggered refresh */
				/* 31:26  25 23	 20 19 18 16 */
				/* 111001 00 111 0  0  00 1 */

				/* refresh count=0x200
				   phy interleave disable (by default,
				   set later by dram config..)
				   virt interleave enable */
				/* 15 14 13:0 */
				/* 1  0	 0x200 */

#define CFG_DEV0_SPACE		CFG_DEV_BASE
#define CFG_DEV1_SPACE		(CFG_DEV0_SPACE + CFG_DEV0_SIZE)
#define CFG_DEV2_SPACE		(CFG_DEV1_SPACE + CFG_DEV1_SIZE)
#define CFG_DEV3_SPACE		(CFG_DEV2_SPACE + CFG_DEV2_SIZE)

/*-----------------------------------------------------------------------
 * PCI stuff
 */

#define PCI_HOST_ADAPTER	0	/* configure ar pci adapter	*/
#define PCI_HOST_FORCE		1	/* configure as pci host	*/
#define PCI_HOST_AUTO		2	/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/

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
#define CFG_PCI0_IO_SPACE_PCI	0x00000000
#define CFG_PCI1_IO_SPACE	(CFG_PCI1_IO_BASE)
#define CFG_PCI1_IO_SPACE_PCI	0x00000000


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
#define CFG_IBAT1U  (CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_DBAT1L  CFG_IBAT1L
#define CFG_DBAT1U  CFG_IBAT1U

/* PCI0, PCI1 memory space (starting at PCI0 base, mapped in one BAT) */
#define CFG_IBAT2L BATL_NO_ACCESS
#define CFG_IBAT2U CFG_DBAT2U
#define CFG_DBAT2L (CFG_PCI0_MEM_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CFG_DBAT2U (CFG_PCI0_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* GT regs, bootrom, all the devices, PCI I/O */
#define CFG_IBAT3L (CFG_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW)
#define CFG_IBAT3U (CFG_MISC_REGION_BASE | BATU_VS | BATU_VP | BATU_BL_256M)
#define CFG_DBAT3L (CFG_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U CFG_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8<<20) /* Initial Memory map for Linux */


/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	3	/* max number of memory banks	*/
#define CFG_MAX_FLASH_SECT	130	/* max number of sectors on one chip */

#define CFG_EXTRA_FLASH_DEVICE	DEVICE0 /* extra flash at device 0 */
#define CFG_EXTRA_FLASH_WIDTH	2	/* 16 bit */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */
#define CFG_FLASH_CFI		1

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CFG_ENV_SECT_SIZE	0x10000 /* see README - env sect real size */
#define CFG_ENV_ADDR		(0xfff80000 - CFG_ENV_SECT_SIZE)

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For all MPC74xx CPUs		 */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/74xx_7xx.h for the defines used here
 */

#define CFG_L2

#ifdef CONFIG_750CX
#define L2_INIT		0
#else
#define L2_INIT		(L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
			L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
#endif

#define L2_ENABLE	(L2_INIT | L2CR_L2E)

/*------------------------------------------------------------------------
 * Real time clock
 */
#define CONFIG_RTC_DS1302


/*------------------------------------------------------------------------
 * Galileo I2C driver
 */
#define CONFIG_GT_I2C

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot		    */

#endif	/* __CONFIG_H */
