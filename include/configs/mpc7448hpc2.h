/*
 * Copyright (c) 2005 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2006
 * Alex Bounine , Tundra Semiconductor Corp.
 * Roy Zang	, <tie-fei.zang@freescale.com> Freescale Corp.
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
 * board specific configuration options for Freescale
 * MPC7448HPC2 (High-Performance Computing II) (Taiga) board
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef DEBUG

/* Board Configuration Definitions */
/* MPC7448HPC2 (High-Performance Computing II) (Taiga) board */

#define CONFIG_MPC7448HPC2

#define CONFIG_74xx
#define CONFIG_750FX		/* this option to enable init of extended BATs */
#define CONFIG_ALTIVEC		/* undef to disable */

#define CFG_BOARD_NAME		"MPC7448 HPC II"
#define CONFIG_IDENT_STRING	" Freescale MPC7448 HPC II"

#define CFG_OCN_CLK		133000000	/* 133 MHz */
#define CFG_CONFIG_BUS_CLK	133000000

#define CFG_CLK_SPREAD		/* Enable Spread-Spectrum Clock generation */

#undef  CONFIG_ECC		/* disable ECC support */

/* Board-specific Initialization Functions to be called */
#define CFG_BOARD_ASM_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_MISC_INIT_R

#define CONFIG_HAS_ETH1

#define CONFIG_ENV_OVERWRITE

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115000 */

/*#define CFG_HUSH_PARSER */
#undef CFG_HUSH_PARSER

#define CFG_PROMPT_HUSH_PS2	"> "

/* Pass open firmware flat tree */
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

#define OF_CPU			"PowerPC,7448@0"
#define OF_TSI			"tsi108@c0000000"
#define OF_TBCLK		(bd->bi_busfreq / 8)
#define OF_STDOUT_PATH		"/tsi108@c0000000/serial@7808"

/*
 * The following defines let you select what serial you want to use
 * for your console driver.
 *
 * what to do:
 * If you have hacked a serial cable onto the second DUART channel,
 * change the CFG_DUART port from 1 to 0 below.
 *
 */

#define CONFIG_CONS_INDEX	1
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		CFG_OCN_CLK * 8

#define CFG_NS16550_COM1	(CFG_TSI108_CSR_RST_BASE+0x7808)
#define CFG_NS16550_COM2	(CFG_TSI108_CSR_RST_BASE+0x7C08)
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds */
#define CONFIG_ZERO_BOOTDELAY_CHECK

#undef CONFIG_BOOTARGS
/* #define CONFIG_PREBOOT  "echo;echo Type \"run flash_nfs\"
 * to mount root filesystem over NFS;echo" */

#if (CONFIG_BOOTDELAY >= 0)
#define CONFIG_BOOTCOMMAND	"tftpboot 0x400000 zImage.initrd.elf;\
 setenv bootargs $(bootargs) $(bootargs_root) nfsroot=$(serverip):$(rootpath) \
 ip=$(ipaddr):$(serverip)$(bootargs_end);  bootm 0x400000; "

#define CONFIG_BOOTARGS "console=ttyS0,115200"
#endif

#undef CONFIG_EXTRA_ENV_SETTINGS

#define CONFIG_SERIAL	"No. 1"

/* Networking Configuration */

#define KSEG1ADDR(a)	(a)	/* Needed by the rtl8139 driver */

#define CONFIG_TSI108_ETH
#define CONFIG_TSI108_ETH_NUM_PORTS	2

#define CONFIG_NET_MULTI

#define CONFIG_BOOTFILE		zImage.initrd.elf
#define CONFIG_LOADADDR		0x400000

/*-------------------------------------------------------------------------- */

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download */
#define CFG_LOADS_BAUD_CHANGE	/* allow baudrate changes */

#undef CONFIG_WATCHDOG		/* watchdog disabled */

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
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_I2C
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_ENV
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_DATE


/*set date in u-boot*/
#define CONFIG_RTC_M48T35A
#define CFG_NVRAM_BASE_ADDR	0xfc000000
#define CFG_NVRAM_SIZE		0x8000
/*
 * Miscellaneous configurable options
 */
#define CONFIG_VERSION_VARIABLE		1
#define CONFIG_TSI108_I2C

#define CFG_I2C_EEPROM_ADDR		0x50	/* I2C EEPROM page 1 */
#define CFG_I2C_EEPROM_ADDR_LEN		1	/* Bytes of address */

#define CFG_LONGHELP		/* undef to save memory */
#define CFG_PROMPT	"=> "	/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE (CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)/* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CFG_MEMTEST_START	0x00400000	/* memtest works on */
#define CFG_MEMTEST_END		0x07c00000	/* 4 ... 124 MB in DRAM */

#define CFG_LOAD_ADDR	0x00400000	/* default load address */

#define CFG_HZ		1000		/* decr freq: 1ms ticks */

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
#undef  CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0x07d00000	/* unused memory region */
#define CFG_INIT_RAM_END	0x4000/* larger space - we have SDRAM initialized */

#define CFG_GBL_DATA_SIZE	128/* size in bytes reserved for init data */
#define CFG_GBL_DATA_OFFSET (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */

#define CFG_SDRAM_BASE		0x00000000	/* first 256 MB of SDRAM */
#define CFG_SDRAM1_BASE		0x10000000	/* next 256MB of SDRAM */

#define CFG_SDRAM2_BASE	0x40000000	/* beginning of non-cacheable alias for SDRAM - first 256MB */
#define CFG_SDRAM3_BASE	0x50000000	/* next Non-Cacheable 256MB of SDRAM */

#define CFG_PCI_PFM_BASE	0x80000000	/* Prefetchable (cacheable) PCI/X PFM and SDRAM OCN (128MB+128MB) */

#define CFG_PCI_MEM32_BASE	0xE0000000	/* Non-Cacheable PCI/X MEM and SDRAM OCN (128MB+128MB) */

#define CFG_MISC_REGION_BASE	0xf0000000	/* Base Address for (PCI/X + Flash) region */

#define CFG_FLASH_BASE	0xff000000	/* Base Address of Flash device */
#define CFG_FLASH_BASE2	0xfe000000	/* Alternate Flash Base Address */

#define CONFIG_VERY_BIG_RAM	/* we will use up to 256M memory for cause we are short of BATS */

#define PCI0_IO_BASE_BOOTM	0xfd000000

#define CFG_RESET_ADDRESS	0x3fffff00
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE	TEXT_BASE	/* u-boot code base */
#define CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */

/* Peripheral Device section */

/*
 * Resources on the Tsi108
 */

#define CFG_TSI108_CSR_RST_BASE	0xC0000000	/* Tsi108 CSR base after reset */
#define CFG_TSI108_CSR_BASE	CFG_TSI108_CSR_RST_BASE	/* Runtime Tsi108 CSR base */

#define ENABLE_PCI_CSR_BAR	/* enables access to Tsi108 CSRs from the PCI/X bus */

#undef  DISABLE_PBM

/*
 * PCI stuff
 *
 */

#define CONFIG_PCI		/* include pci support */
#define CONFIG_TSI108_PCI	/* include tsi108 pci support */

#define PCI_HOST_ADAPTER	0	/* configure as pci adapter */
#define PCI_HOST_FORCE		1	/* configure as pci host */
#define PCI_HOST_AUTO		2	/* detected via arbiter enable */

#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function */
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

/* PCI MEMORY MAP section */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS	0x00000000
#define CFG_PCI_MEMORY_PHYS	0x00000000
#define CFG_PCI_MEMORY_SIZE	0x80000000

/* PCI Memory Space */
#define CFG_PCI_MEM_BUS		(CFG_PCI_MEM_PHYS)
#define CFG_PCI_MEM_PHYS	(CFG_PCI_MEM32_BASE)	/* 0xE0000000 */
#define CFG_PCI_MEM_SIZE	0x10000000	/* 256 MB space for PCI/X Mem + SDRAM OCN */

/* PCI I/O Space */
#define CFG_PCI_IO_BUS		0x00000000
#define CFG_PCI_IO_PHYS		0xfa000000	/* Changed from fd000000 */

#define CFG_PCI_IO_SIZE		0x01000000	/* 16MB */

#define _IO_BASE		0x00000000	/* points to PCI I/O space      */

/* PCI Config Space mapping */
#define CFG_PCI_CFG_BASE	0xfb000000	/* Changed from FE000000 */
#define CFG_PCI_CFG_SIZE	0x01000000	/* 16MB */

#define CFG_IBAT0U	0xFE0003FF
#define CFG_IBAT0L	0xFE000002

#define CFG_IBAT1U	0x00007FFF
#define CFG_IBAT1L	0x00000012

#define CFG_IBAT2U	0x80007FFF
#define CFG_IBAT2L	0x80000022

#define CFG_IBAT3U	0x00000000
#define CFG_IBAT3L	0x00000000

#define CFG_IBAT4U	0x00000000
#define CFG_IBAT4L	0x00000000

#define CFG_IBAT5U	0x00000000
#define CFG_IBAT5L	0x00000000

#define CFG_IBAT6U	0x00000000
#define CFG_IBAT6L	0x00000000

#define CFG_IBAT7U	0x00000000
#define CFG_IBAT7L	0x00000000

#define CFG_DBAT0U	0xE0003FFF
#define CFG_DBAT0L	0xE000002A

#define CFG_DBAT1U	0x00007FFF
#define CFG_DBAT1L	0x00000012

#define CFG_DBAT2U	0x00000000
#define CFG_DBAT2L	0x00000000

#define CFG_DBAT3U	0xC0000003
#define CFG_DBAT3L	0xC000002A

#define CFG_DBAT4U	0x00000000
#define CFG_DBAT4L	0x00000000

#define CFG_DBAT5U	0x00000000
#define CFG_DBAT5L	0x00000000

#define CFG_DBAT6U	0x00000000
#define CFG_DBAT6L	0x00000000

#define CFG_DBAT7U	0x00000000
#define CFG_DBAT7L	0x00000000

/* I2C addresses for the two DIMM SPD chips */
#define DIMM0_I2C_ADDR	0x51
#define DIMM1_I2C_ADDR	0x52

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8<<20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1/* Flash can be at one of two addresses */
#define FLASH_BANK_SIZE		0x01000000	/* 16 MB Total */
#define CFG_FLASH_BANKS_LIST {CFG_FLASH_BASE, CFG_FLASH_BASE2}

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_WRITE_SWAPPED_DATA

#define PHYS_FLASH_SIZE		0x01000000
#define CFG_MAX_FLASH_SECT	(128)

#define CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_ADDR		0xFC000000

#define CFG_ENV_OFFSET	0x00000000	/* Offset of Environment Sector */
#define CFG_ENV_SIZE	0x00000400	/* Total Size of Environment Space */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For all MPC74xx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/mpc74xx.h for the defines used here
 */
#undef CFG_L2

#define L2_INIT		0
#define L2_ENABLE	(L2_INIT | L2CR_L2E)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	/* Software reboot */
#define CFG_SERIAL_HANG_IN_EXCEPTION
#endif	/* __CONFIG_H */
