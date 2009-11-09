/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Copyright (C) 2009
 * Albin Tonnerre, Free Electrons <albin.tonnerre@free-electrons.com>
 *
 * Configuation settings for the Calao TNY-A9260 and TNY-A9G20 boards
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#if defined(CONFIG_TNY_A9260_NANDFLASH) || defined(CONFIG_TNY_A9260_EEPROM)
#define CONFIG_TNY_A9260
#elif defined(CONFIG_TNY_A9G20_NANDFLASH) || defined(CONFIG_TNY_A9G20_EEPROM)
#define CONFIG_TNY_A9G20
#endif

#ifdef CONFIG_TNY_A9260
#define CONFIG_AT91SAM9260
#else
#define CONFIG_AT91SAM9G20
#endif

#if defined(CONFIG_TNY_A9260_NANDFLASH) || defined(CONFIG_TNY_A9G20_NANDFLASH)
#define CONFIG_ENV_IS_IN_NAND
#else
#define CONFIG_ENV_IS_IN_EEPROM
#endif

/* ARM asynchronous clock */
#define AT91_MAIN_CLOCK		12000000	/* 12 MHz crystal */
#define CONFIG_SYS_HZ		1000

#define CONFIG_ARM926EJS	1	/* This is an ARM926EJS Core */
#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Hardware drivers
 */
#define CONFIG_ATMEL_USART	1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2
#define CONFIG_USART3		1	/* USART 3 is DBGU */

#define CONFIG_BOOTDELAY	3

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_SOURCE
#undef CONFIG_CMD_USB

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x20000000
#define PHYS_SDRAM_SIZE			0x04000000	/* 64 megs */

/* SPI EEPROM */
#define CONFIG_SPI
#define CONFIG_CMD_SPI
#define CONFIG_ATMEL_SPI
#define CONFIG_SYS_SPI_WRITE_TOUT		(5 * CONFIG_SYS_HZ)

#define CONFIG_CMD_EEPROM
#define CONFIG_SPI_M95XXX
#define CONFIG_SYS_EEPROM_SIZE 0x10000
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5

/* NAND flash */
#define CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8			1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC13

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH			1

#define CONFIG_DOS_PARTITION		1
#define CONFIG_CMD_FAT			1

#define CONFIG_SYS_LOAD_ADDR			0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x23e00000

/* Env in EEPROM, bootstrap + u-boot in NAND*/
#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0x20
#define CONFIG_ENV_SIZE		0x1000
#endif

/* Env, bootstrap and u-boot in NAND */
#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET 0x60000
#define CONFIG_ENV_OFFSET_REDUND 0x80000
#define CONFIG_ENV_SIZE 0x20000
#define CONFIG_SYS_64BIT_VSPRINTF	/* needed for nand_util.c */
#endif

#define CONFIG_BOOTCOMMAND	"nboot 0x21000000 0 400000"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 " \
				"root=/dev/mtdblock1 " \
				"mtdparts=atmel_nand:16M(kernel)ro," \
				"120M(rootfs),-(other) " \
				"rw rootfstype=jffs2"

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP	1
#define CONFIG_CMDLINE_EDITING	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_STACKSIZE	(32 * 1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
