/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9260EK & AT91SAM9G20EK boards.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* main clock xtal */

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		0x04000000

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
#ifdef CONFIG_AT91SAM9XE
# define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#else
# define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM1 + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13
#endif

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000	/* AT91SAM9260_UHP_BASE */
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

#endif
