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

#define CONFIG_SYS_GT_6426x        GT_64260 /* with a 64260 system controller */
#define CONFIG_ETHER_PORT_MII	/* use two MII ports */
#define CONFIG_INTEL_LXT97X	/* Intel LXT97X phy */

#ifndef __ASSEMBLY__
#include <galileo/core.h>
#endif

#include "../board/evb64260/local.h"

#define CONFIG_EVB64260		1	/* this is an EVB64260 board	*/
#define CONFIG_ZUMA_V2		1	/* always define this for ZUMA v2 */

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

/* #define CONFIG_ZUMA_V2_OLD	1 */	/* backwards compat for old V2 board */

#define CONFIG_BAUDRATE		38400	/* console baudrate = 38400	*/

#define CONFIG_ECC			/* enable ECC support */

#define CONFIG_750CX			/* we have a 750CX/CXe (override local.h) */

/* which initialization functions to call for this board */
#define CONFIG_MISC_INIT_R
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_BOARD_ASM_INIT

#define CONFIG_SYS_BOARD_NAME		"Zuma APv2"

#define CONFIG_SYS_HUSH_PARSER

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
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor1=zuma-1,nor2=zuma-2"
#define MTDPARTS_DEFAULT	"mtdparts=zuma-1:-(jffs2),zuma-2:-(user)"
*/

/*
 * Miscellaneous configurable options
 */
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

#define CONFIG_SYS_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00300000	/* default load address */

#define CONFIG_SYS_HZ			1000		/* decr freq: 1ms ticks */

#define CONFIG_SYS_BUS_CLK		133000000	/* 133 MHz		*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_RAM_LOCK


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xfff00000
#define CONFIG_SYS_RESET_ADDRESS	0xfff00100
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */

/* areas to map different things with the GT in physical space */
#define CONFIG_SYS_DRAM_BANKS		4
#define CONFIG_SYS_DFL_GT_REGS		0x14000000	/* boot time GT_REGS */

/* What to put in the bats. */
#define CONFIG_SYS_MISC_REGION_BASE	0xf0000000

/* Peripheral Device section */
#define CONFIG_SYS_GT_REGS		0xf8000000	/* later mapped GT_REGS */
#define CONFIG_SYS_DEV_BASE		0xf0000000
#define CONFIG_SYS_DEV0_SIZE		_64M /* zuma flash @ 0xf000.0000*/
#define CONFIG_SYS_DEV1_SIZE		 _8M /* zuma IDE   @ 0xf400.0000 */
#define CONFIG_SYS_DEV2_SIZE		 _8M /* unused */
#define CONFIG_SYS_DEV3_SIZE		 _8M /* unused */

#define CONFIG_SYS_DEV0_PAR		0xc498243c
	/*     c    4     9     8     2	    4     3     c */
	/* 33 22|2222|22 22|111 1|11 11|1 1  |	   |      */
	/* 10 98|7654|32 10|987 6|54 32|1 098|7 654|3 210 */
	/* 11|00|0100|10 01|100|0 00|10 0|100 0|011 1|100 */
	/*  3| 0|.... ..| 1| 4 |  0 |  4 |   8 |   7 | 4  */

#define CONFIG_SYS_DEV1_PAR		0xc01b6ac5
	/*     c    0     1     b     6	    a     c     5 */
	/* 33 22|2222|22 22|111 1|11 11|1 1  |	   |      */
	/* 10 98|7654|32 10|987 6|54 32|1 098|7 654|3 210 */
	/* 11|00|0000|00 01|101|1 01|10 1|010 1|100 0|101 */
	/*  3| 0|.... ..| 1| 5 |  5 |  5 |   5 |   8 | 5  */


#define CONFIG_SYS_8BIT_BOOT_PAR	0xc00b5e7c

#define CONFIG_SYS_MPP_CONTROL_0	0x00007777 /* GPP[7:4] : REQ0[1:0] GNT0[1:0] */
#define CONFIG_SYS_MPP_CONTROL_1	0x00000000 /* GPP[15:12] : GPP[11:8] */
#define CONFIG_SYS_MPP_CONTROL_2	0x00008888 /* GPP[23:20] : REQ1[1:0] GNT1[1:0] */
#define CONFIG_SYS_MPP_CONTROL_3	0x00000000 /* GPP[31:28] (int[3:0]) */
					   /* GPP[27:24] (27 is int4, rest are GPP) */

#define CONFIG_SYS_SERIAL_PORT_MUX	0x00001101 /* 11=MPSC1/MPSC0 01=ETH,  0=only MII */
#define CONFIG_SYS_GPP_LEVEL_CONTROL	0xf8000000 /* interrupt inputs: GPP[31:27] */

#define CONFIG_SYS_SDRAM_CONFIG	0xe4e18200	/* 0x448 */
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

#define CONFIG_SYS_DEV0_SPACE		CONFIG_SYS_DEV_BASE
#define CONFIG_SYS_DEV1_SPACE		(CONFIG_SYS_DEV0_SPACE + CONFIG_SYS_DEV0_SIZE)
#define CONFIG_SYS_DEV2_SPACE		(CONFIG_SYS_DEV1_SPACE + CONFIG_SYS_DEV1_SIZE)
#define CONFIG_SYS_DEV3_SPACE		(CONFIG_SYS_DEV2_SPACE + CONFIG_SYS_DEV2_SIZE)

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
#define CONFIG_SYS_IBAT1U  (CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L  CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U  CONFIG_SYS_IBAT1U

/* PCI0, PCI1 memory space (starting at PCI0 base, mapped in one BAT) */
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
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8<<20) /* Initial Memory map for Linux */


/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* max number of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	130	/* max number of sectors on one chip */

#define CONFIG_SYS_EXTRA_FLASH_DEVICE	DEVICE0 /* extra flash at device 0 */
#define CONFIG_SYS_EXTRA_FLASH_WIDTH	2	/* 16 bit */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */
#define CONFIG_SYS_FLASH_CFI		1

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x10000 /* see README - env sect real size */
#define CONFIG_ENV_ADDR		(0xfff80000 - CONFIG_ENV_SECT_SIZE)

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For all MPC74xx CPUs		 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/74xx_7xx.h for the defines used here
 */

#define CONFIG_SYS_L2

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

#endif	/* __CONFIG_H */
