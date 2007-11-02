/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
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

#ifndef _M5253EVBE_H
#define _M5253EVBE_H

#define CONFIG_MCF52x2		/* define processor family */
#define CONFIG_M5253		/* define processor type */
#define CONFIG_M5253EVBE	/* define board type */

#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)
#define CONFIG_BAUDRATE		19200
#define CFG_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

#undef CONFIG_WATCHDOG		/* disable watchdog */

#define CONFIG_BOOTDELAY	5

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#ifndef CONFIG_MONITOR_IS_IN_RAM
#define CFG_ENV_OFFSET		0x4000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#else
#define CFG_ENV_ADDR		0xffe04000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#endif

/*
 * BOOTP options
 */
#undef CONFIG_BOOTP_BOOTFILESIZE
#undef CONFIG_BOOTP_BOOTPATH
#undef CONFIG_BOOTP_GATEWAY
#undef CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_NET
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC

/* ATA */
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_IDE_RESET	1
#define CONFIG_IDE_PREINIT	1
#define CONFIG_ATAPI
#undef CONFIG_LBA48

#define CFG_IDE_MAXBUS		1
#define CFG_IDE_MAXDEVICE	2

#define CFG_ATA_BASE_ADDR	(CFG_MBAR2 + 0x800)
#define CFG_ATA_IDE0_OFFSET	0

#define CFG_ATA_DATA_OFFSET	0xA0	/* Offset for data I/O */
#define CFG_ATA_REG_OFFSET	0xA0	/* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0xC0	/* Offset for alternate registers */
#define CFG_ATA_STRIDE		4	/* Interval between registers */
#define _IO_BASE		0

#define CFG_PROMPT		"=> "
#define CFG_LONGHELP		/* undef to save memory */

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CFG_LOAD_ADDR		0x00100000

#define CFG_MEMTEST_START	0x400
#define CFG_MEMTEST_END		0x380000

#define CFG_HZ			1000

#undef CFG_PLL_BYPASS		/* bypass PLL for test purpose */
#define CFG_FAST_CLK
#ifdef CFG_FAST_CLK
#	define CFG_PLLCR	0x1243E054
#	define CFG_CLK		140000000
#else
#	define CFG_PLLCR	0x135a4140
#	define CFG_CLK		70000000
#endif

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CFG_MBAR		0x10000000	/* Register Base Addrs */
#define CFG_MBAR2		0x80000000	/* Module Base Addrs 2 */

/*
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x20000000
#define CFG_INIT_RAM_END	0x10000	/* End of used area in internal SRAM */
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_SIZE		8	/* SDRAM size in MB */

#ifdef CONFIG_MONITOR_IS_IN_RAM
#define CFG_MONITOR_BASE	0x20000
#else
#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)
#endif

#define CFG_MONITOR_LEN		0x40000
#define CFG_MALLOC_LEN		(256 << 10)
#define CFG_BOOTPARAMS_LEN	(64*1024)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_BOOTMAPSZ		(CFG_SDRAM_BASE + (CFG_SDRAM_SIZE << 20))

/* FLASH organization */
#define CFG_FLASH_BASE		0xffe00000
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	35	/* max number of sectors on one chip */
#define CFG_FLASH_ERASE_TOUT	1000

#define CFG_FLASH_CFI		1
#define CFG_FLASH_CFI_DRIVER	1
#define CFG_FLASH_SIZE		0x200000
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

/* Cache Configuration */
#define CFG_CACHELINE_SIZE	16

/* Port configuration */
#define CFG_FECI2C		0xF0

#define CFG_CSAR0		0xFFE0
#define CFG_CSMR0		0x001F0021
#define CFG_CSCR0		0x1D80

#define CFG_CSAR1		0
#define CFG_CSMR1		0
#define CFG_CSCR1		0

#define CFG_CSAR2		0
#define CFG_CSMR2		0
#define CFG_CSCR2		0

#define CFG_CSAR3		0
#define CFG_CSMR3		0
#define CFG_CSCR3		0

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CFG_GPIO_FUNC		0x00000008	/* Set gpio pins: none */
#define CFG_GPIO1_FUNC		0x00df00f0	/* 36-39(SWITCH),48-52(FPGAs),54 */
#define CFG_GPIO_EN		0x00000008	/* Set gpio output enable */
#define CFG_GPIO1_EN		0x00c70000	/* Set gpio output enable */
#define CFG_GPIO_OUT		0x00000008	/* Set outputs to default state */
#define CFG_GPIO1_OUT		0x00c70000	/* Set outputs to default state */
#define CFG_GPIO1_LED		0x00400000	/* user led */

#endif				/* _M5253EVB_H */
