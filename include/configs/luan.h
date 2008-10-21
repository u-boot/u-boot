/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 * John Otken, jotken@softadvances.com
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

/************************************************************************
 * luan.h - configuration for LUAN board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_LUAN		1	/* Board is Luan		*/
#define CONFIG_440SP		1	/* Specific PPC440SP support    */
#define CONFIG_4xx		1	/* PPC4xx family	        */
#define CONFIG_440		1
#define CONFIG_SYS_CLK_FREQ	33333333 /* external freq to pll	*/

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		luan
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_LARGE_FLASH		0xffc00000	/* 4MB flash address CS0 */
#define CONFIG_SYS_SMALL_FLASH		0xff900000	/* 1MB flash address CS2 */
#define CONFIG_SYS_SRAM_BASE		0xff800000	/* 1MB SRAM  address CS2 */
#define CONFIG_SYS_EPLD_BASE		0xff000000	/* EPLD and FRAM     CS1 */

#define CONFIG_SYS_ISRAM_BASE	        0xf8000000	/* internal 8k SRAM (L2 cache) */

#define CONFIG_SYS_PERIPHERAL_BASE     0xf0000000	/* internal peripherals */

#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory */
#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs */
#define CONFIG_SYS_PCI_TARGBASE	0x80000000	/* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

#if CONFIG_SYS_LARGE_FLASH == 0xffc00000
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_LARGE_FLASH
#else
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_SMALL_FLASH
#endif

#if CONFIG_SYS_SRAM_BASE
#define CONFIG_SYS_KBYTES_SDRAM	1024*2
#else
#define CONFIG_SYS_KBYTES_SDRAM	1024
#endif

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in SDRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_END	(8 << 10)
#define CONFIG_SYS_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_EXT_SERIAL_CLOCK	11059200 /* external 11.059MHz clk */
#undef  CONFIG_UART1_CONSOLE		/* define if you want console on UART1 */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH or EEPROM).
 * Note: DENX encourages to use redundant environment in FLASH.
 */
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CONFIG_SYS_FLASH_ADDR0         0x555
#define CONFIG_SYS_FLASH_ADDR1         0x2aa
#define CONFIG_SYS_FLASH_WORD_SIZE     unsigned char

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000 /* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{0x53, 0x52}	/* SPD i2c spd addresses*/
#define CONFIG_DDR_ECC		1	/* with ECC support		*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/

#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR	(0xa8>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_PPC						\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc100000\0"					\
	""

#define CONFIG_HAS_ETH0
#define CONFIG_PHY_ADDR		1
#define CONFIG_CIS8201_PHY	1	/* Enable 'special' RGMII mode for Cicada phy */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#ifdef DEBUG
#define CONFIG_PANIC_HANG
#else
#define CONFIG_HW_WATCHDOG			/* watchdog */
#endif

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_PCI
#define CONFIG_CMD_SDRAM

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#if defined(CONFIG_CMD_PCI)

/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#undef  CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x4403	/* whatever */

#endif

#endif	/* __CONFIG_H */
