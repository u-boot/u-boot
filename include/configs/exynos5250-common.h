
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5250 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_5250_H
#define __CONFIG_5250_H

#include <configs/exynos5-common.h>
#define CONFIG_EXYNOS5250

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x43E00000

/* MACH_TYPE_SMDK5250 macro will be removed once added to mach-types */
#define MACH_TYPE_SMDK5250		3774
#define CONFIG_MACH_TYPE		MACH_TYPE_SMDK5250

#define CONFIG_SPL_MAX_FOOTPRINT	(14 * 1024)

/* USB */
#define CONFIG_CMD_USB
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE

#define CONFIG_SPL_TEXT_BASE	0x02023400

#define CONFIG_BOOTCOMMAND	"mmc read 40007000 451 2000; bootm 40007000"

#define CONFIG_IRAM_STACK	0x02050000

#define CONFIG_SYS_INIT_SP_ADDR	CONFIG_IRAM_STACK

/* PMIC */
#define CONFIG_POWER_MAX77686

/* Sound */
#define CONFIG_CMD_SOUND
#ifdef CONFIG_CMD_SOUND
#define CONFIG_SOUND
#define CONFIG_I2S_SAMSUNG
#define CONFIG_I2S
#define CONFIG_SOUND_MAX98095
#define CONFIG_SOUND_WM8994
#endif

/* I2C */
#define CONFIG_MAX_I2C_NUM	8

/* Display */
#define CONFIG_LCD
#ifdef CONFIG_LCD
#define CONFIG_EXYNOS_FB
#define CONFIG_EXYNOS_DP
#define LCD_BPP			LCD_COLOR16
#endif

/* DRAM Memory Banks */
#define CONFIG_NR_DRAM_BANKS	8
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */

#endif  /* __CONFIG_5250_H */
