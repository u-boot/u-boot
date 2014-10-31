/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Copyright (C) 2009
 * Albin Tonnerre, Free Electrons <albin.tonnerre@free-electrons.com>
 *
 * Configuation settings for the Calao TNY-A9260 and TNY-A9G20 boards
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

#if defined(CONFIG_TNY_A9260_NANDFLASH) || defined(CONFIG_TNY_A9G20_NANDFLASH)
#define CONFIG_ENV_IS_IN_NAND
#else
#define CONFIG_ENV_IS_IN_EEPROM
#endif

/* Define actual evaluation board type from used processor type */
#ifdef CONFIG_AT91SAM9G20
# define CONFIG_TNY_A9G20
# define MACH_TYPE_TNY_A9G20		2059
# define CONFIG_MACH_TYPE		MACH_TYPE_TNY_A9G20
#else
# define CONFIG_TNY_A9260
# define MACH_TYPE_TNY_A9260		2058
# define CONFIG_MACH_TYPE		MACH_TYPE_TNY_A9260
#endif

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_CMDLINE_TAG	        /* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Hardware drivers
 */
#define CONFIG_ATMEL_LEGACY
#define CONFIG_AT91_GPIO

#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define	CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

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
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SOURCE
#undef CONFIG_CMD_USB

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		0x04000000	/* 64 megs */
# define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM1 + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* SPI EEPROM */
#define CONFIG_SPI
#define CONFIG_CMD_SPI
#define CONFIG_ATMEL_SPI

#define CONFIG_CMD_EEPROM
#define CONFIG_SPI_M95XXX
#define CONFIG_SYS_EEPROM_SIZE 0x10000
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5

/* NAND flash */
#define CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC13

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT

#define CONFIG_SYS_LOAD_ADDR			0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			0x23e00000

/* Env in EEPROM, bootstrap + u-boot in NAND*/
#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0x20
#define CONFIG_ENV_SIZE		        0x1000
#endif

/* Env, bootstrap and u-boot in NAND */
#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET               0x60000
#define CONFIG_ENV_OFFSET_REDUND        0x80000
#define CONFIG_ENV_SIZE                 0x20000
#endif

#define CONFIG_BOOTCOMMAND	"nboot 0x21000000 0 400000"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 " \
				"root=/dev/mtdblock1 " \
				"mtdparts=atmel_nand:16M(kernel)ro," \
				"120M(rootfs),-(other) " \
				"rw rootfstype=jffs2"

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024, 0x1000)

#endif
