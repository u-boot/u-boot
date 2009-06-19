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

#ifndef _M5253DEMO_H
#define _M5253DEMO_H

#define CONFIG_MCF52x2		/* define processor family */
#define CONFIG_M5253		/* define processor type */
#define CONFIG_M5253DEMO	/* define board type */

#define CONFIG_MCFTMR

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

#undef CONFIG_WATCHDOG		/* disable watchdog */

#define CONFIG_BOOTDELAY	5

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#ifdef CONFIG_MONITOR_IS_IN_RAM
#	define CONFIG_ENV_OFFSET		0x4000
#	define CONFIG_ENV_SECT_SIZE	0x1000
#	define CONFIG_ENV_IS_IN_FLASH	1
#else
#	define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x4000)
#	define CONFIG_ENV_SECT_SIZE	0x1000
#	define CONFIG_ENV_IS_IN_FLASH	1
#endif

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_PING

#ifdef CONFIG_CMD_IDE
/* ATA */
#	define CONFIG_DOS_PARTITION
#	define CONFIG_MAC_PARTITION
#	define CONFIG_IDE_RESET		1
#	define CONFIG_IDE_PREINIT	1
#	define CONFIG_ATAPI
#	undef CONFIG_LBA48

#	define CONFIG_SYS_IDE_MAXBUS		1
#	define CONFIG_SYS_IDE_MAXDEVICE	2

#	define CONFIG_SYS_ATA_BASE_ADDR	(CONFIG_SYS_MBAR2 + 0x800)
#	define CONFIG_SYS_ATA_IDE0_OFFSET	0

#	define CONFIG_SYS_ATA_DATA_OFFSET	0xA0	/* Offset for data I/O */
#	define CONFIG_SYS_ATA_REG_OFFSET	0xA0	/* Offset for normal register accesses */
#	define CONFIG_SYS_ATA_ALT_OFFSET	0xC0	/* Offset for alternate registers */
#	define CONFIG_SYS_ATA_STRIDE		4	/* Interval between registers */
#endif

#define CONFIG_NET_MULTI		1
#define CONFIG_DRIVER_DM9000
#ifdef CONFIG_DRIVER_DM9000
#	define CONFIG_DM9000_BASE	(CONFIG_SYS_CS1_BASE | 0x300)
#	define DM9000_IO		CONFIG_DM9000_BASE
#	define DM9000_DATA		(CONFIG_DM9000_BASE + 4)
#	undef CONFIG_DM9000_DEBUG

#	define CONFIG_ETHADDR		00:e0:0c:bc:e5:60
#	define CONFIG_IPADDR		10.82.121.249
#	define CONFIG_NETMASK		255.255.252.0
#	define CONFIG_SERVERIP		10.82.120.80
#	define CONFIG_GATEWAYIP		10.82.123.254
#	define CONFIG_OVERWRITE_ETHADDR_ONCE

#	define CONFIG_EXTRA_ENV_SETTINGS		\
		"netdev=eth0\0"				\
		"inpclk=" MK_STR(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
		"loadaddr=10000\0"			\
		"u-boot=u-boot.bin\0"			\
		"load=tftp ${loadaddr) ${u-boot}\0"	\
		"upd=run load; run prog\0"		\
		"prog=prot off 0 2ffff;"	\
		"era 0 2ffff;"			\
		"cp.b ${loadaddr} 0 ${filesize};"	\
		"save\0"				\
		""
#endif

#define CONFIG_HOSTNAME		M5253DEMO

/* I2C */
#define CONFIG_FSL_I2C
#define CONFIG_HARD_I2C		/* I2C with hw support */
#define CONFIG_SYS_I2C_SPEED		80000
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x00000280
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR
#define CONFIG_SYS_I2C_PINMUX_REG	(*(u32 *) (CONFIG_SYS_MBAR+0x19C))
#define CONFIG_SYS_I2C_PINMUX_CLR	(0xFFFFE7FF)
#define CONFIG_SYS_I2C_PINMUX_SET	(0)

#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_LONGHELP		/* undef to save memory */

#if defined(CONFIG_CMD_KGDB)
#	define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#	define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define CONFIG_SYS_MEMTEST_START	0x400
#define CONFIG_SYS_MEMTEST_END		0x380000

#define CONFIG_SYS_HZ			1000

#undef CONFIG_SYS_PLL_BYPASS		/* bypass PLL for test purpose */
#define CONFIG_SYS_FAST_CLK
#ifdef CONFIG_SYS_FAST_CLK
#	define CONFIG_SYS_PLLCR	0x1243E054
#	define CONFIG_SYS_CLK		140000000
#else
#	define CONFIG_SYS_PLLCR	0x135a4140
#	define CONFIG_SYS_CLK		70000000
#endif

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

#define CONFIG_SYS_MBAR		0x10000000	/* Register Base Addrs */
#define CONFIG_SYS_MBAR2		0x80000000	/* Module Base Addrs 2 */

/*
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000
#define CONFIG_SYS_INIT_RAM_END	0x10000	/* End of used area in internal SRAM */
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		16	/* SDRAM size in MB */

#ifdef CONFIG_MONITOR_IS_IN_RAM
#	define CONFIG_SYS_MONITOR_BASE	0x20000
#else
#	define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#endif

#define CONFIG_SYS_MONITOR_LEN		0x40000
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
#define CONFIG_SYS_BOOTPARAMS_LEN	(64*1024)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))
#define CONFIG_SYS_BOOTM_LEN		(CONFIG_SYS_SDRAM_SIZE << 20)

/* FLASH organization */
#define CONFIG_SYS_FLASH_BASE		(CONFIG_SYS_CS0_BASE)
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	2048	/* max number of sectors on one chip */
#define CONFIG_SYS_FLASH_ERASE_TOUT	1000

#define FLASH_SST6401B		0x200
#define SST_ID_xF6401B		0x236D236D

#undef CONFIG_SYS_FLASH_CFI
#ifdef CONFIG_SYS_FLASH_CFI
/*
 * Unable to use CFI driver, due to incompatible sector erase command by SST.
 * Amd/Atmel use 0x30 for sector erase, SST use 0x50.
 * 0x30 is block erase in SST
 */
#	define CONFIG_FLASH_CFI_DRIVER	1
#	define CONFIG_SYS_FLASH_SIZE		0x800000
#	define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CONFIG_FLASH_CFI_LEGACY
#else
#	define CONFIG_SYS_SST_SECT		2048
#	define CONFIG_SYS_SST_SECTSZ		0x1000
#	define CONFIG_SYS_FLASH_WRITE_TOUT	500
#endif

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE	16

/* Port configuration */
#define CONFIG_SYS_FECI2C		0xF0

#define CONFIG_SYS_CS0_BASE		0xFF800000
#define CONFIG_SYS_CS0_MASK		0x007F0021
#define CONFIG_SYS_CS0_CTRL		0x00001D80

#define CONFIG_SYS_CS1_BASE		0xE0000000
#define CONFIG_SYS_CS1_MASK		0x00000001
#define CONFIG_SYS_CS1_CTRL		0x00003DD8

/*-----------------------------------------------------------------------
 * Port configuration
 */
#define CONFIG_SYS_GPIO_FUNC		0x00000008	/* Set gpio pins: none */
#define CONFIG_SYS_GPIO1_FUNC		0x00df00f0	/* 36-39(SWITCH),48-52(FPGAs),54 */
#define CONFIG_SYS_GPIO_EN		0x00000008	/* Set gpio output enable */
#define CONFIG_SYS_GPIO1_EN		0x00c70000	/* Set gpio output enable */
#define CONFIG_SYS_GPIO_OUT		0x00000008	/* Set outputs to default state */
#define CONFIG_SYS_GPIO1_OUT		0x00c70000	/* Set outputs to default state */
#define CONFIG_SYS_GPIO1_LED		0x00400000	/* user led */

#endif				/* _M5253DEMO_H */
