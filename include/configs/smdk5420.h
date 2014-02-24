/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5420 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_5420_H
#define __CONFIG_5420_H

#include <configs/exynos5-dt.h>

#define CONFIG_EXYNOS5420		/* which is in a Exynos5 Family */
#define CONFIG_SMDK5420			/* which is in a SMDK5420 */

#undef CONFIG_DEFAULT_DEVICE_TREE
#define CONFIG_DEFAULT_DEVICE_TREE	exynos5420-smdk5420

#define CONFIG_VAR_SIZE_SPL

#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_TEXT_BASE		0x23E00000

#define CONFIG_BOARD_REV_GPIO_COUNT	2

/* MACH_TYPE_SMDK5420 macro will be removed once added to mach-types */
#define MACH_TYPE_SMDK5420		8002 /* Temporary number */
#define CONFIG_MACH_TYPE		MACH_TYPE_SMDK5420

/* select serial console configuration */
#define CONFIG_SERIAL3			/* use SERIAL 3 */

#ifdef CONFIG_VAR_SIZE_SPL
#define CONFIG_SPL_TEXT_BASE		0x02024410
#else
#define CONFIG_SPL_TEXT_BASE	0x02024400
#endif

#define CONFIG_BOOTCOMMAND	"mmc read 20007000 451 2000; bootm 20007000"

#define CONFIG_SYS_PROMPT		"SMDK5420 # "
#define CONFIG_IDENT_STRING		" for SMDK5420"

#define CONFIG_IRAM_TOP		0x02074000
/*
 * Put the initial stack pointer 1KB below this to allow room for the
 * SPL marker. This value is arbitrary, but gd_t is placed starting here.
 */
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_IRAM_TOP - 0x800)

#define CONFIG_MAX_I2C_NUM	11

#endif	/* __CONFIG_5420_H */
