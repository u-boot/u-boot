/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2010
 * Achim Ehrlich <aehrlich@taskit.de>
 * taskit GmbH <www.taskit.de>
 *
 * (C) Copyright 2012
 * Markus Hubig <mhubig@imko.de>
 * IMKO GmbH <www.imko.de>
 *
 * (C) Copyright 2014
 * Heiko Schocher <hs@denx.de>
 * DENX Software Engineering GmbH
 *
 * Configuation settings for the smartweb.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>
#include <linux/sizes.h>

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires adapting the initial boot
 * program. Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432MHz crystal */

/* misc settings */

/*
 * SDRAM: 1 bank, 64 MB, base address 0x20000000
 * Already initialized before u-boot gets started.
 */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		(64 * SZ_1M)

/*
 * Perform a SDRAM Memtest from the start of SDRAM
 * till the beginning of the U-Boot position in RAM.
 */

/* NAND flash settings */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13

/* serial console */
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS

/* USB DFU support */

#define CONFIG_USB_GADGET_AT91

/* DFU class support */
#define DFU_MANIFEST_POLL_TIMEOUT	25000

/* General Boot Parameter */

/*
 * Predefined environment variables.
 * Usefull to define some easy to use boot commands.
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
									\
	"basicargs=console=ttyS0,115200\0"				\
									\

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above that
 * address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM1

/* Defines for SPL */

#define CONFIG_SYS_NAND_ENABLE_PIN_SPL	(2*32 + 14)
#define CONFIG_SYS_NAND_U_BOOT_SIZE	SZ_512K
#define	CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_SIZE		(SZ_256M)
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }

#define CONFIG_SYS_MASTER_CLOCK		(198656000/2)
#define AT91_PLL_LOCK_TIMEOUT		1000000
#define CONFIG_SYS_AT91_PLLA		0x2060bf09
#define CONFIG_SYS_MCKR			0x100
#define CONFIG_SYS_MCKR_CSS		(0x02 | CONFIG_SYS_MCKR)
#define CONFIG_SYS_AT91_PLLB		0x10483f0e

#endif /* __CONFIG_H */
