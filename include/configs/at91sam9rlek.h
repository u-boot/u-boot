/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9RLEK board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* main clock xtal */

/*
 * Hardware drivers
 */

/* LCD */
#define LCD_BPP				LCD_COLOR8
/* Let board_init_f handle the framebuffer allocation */
#undef CONFIG_FB_ADDR

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		0x04000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8			1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PB6
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PD17

#endif

#endif
