/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Atmel Corporation
 * Copyright (C) 2019 Stefan Roese <sr@denx.de>
 *
 * Configuation settings for the GARDENA smart Gateway (AT91SAM9G25)
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000	/* 128 megs */

/* NAND flash */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	BIT(21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	BIT(22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PD4
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PD5

/* SPL */

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#define CONFIG_SYS_NAND_U_BOOT_SIZE	0xa0000
#define	CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE

#endif
