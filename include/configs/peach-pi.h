/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG/GOOGLE PEACH-PI board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_PEACH_PI_H
#define __CONFIG_PEACH_PI_H

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SPI_FLASH
#define CONFIG_ENV_SPI_BASE	0x12D30000
#define FLASH_SIZE		(0x4 << 20)
#define CONFIG_ENV_OFFSET	(FLASH_SIZE - CONFIG_BL2_SIZE)
#define CONFIG_SPI_BOOTING

#include <configs/exynos5420-common.h>
#include <configs/exynos5-dt-common.h>

#define CONFIG_BOARD_COMMON

#define CONFIG_SYS_SDRAM_BASE	0x20000000
#define CONFIG_SYS_TEXT_BASE	0x23E00000
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_IRAM_TOP - 0x800)

/* select serial console configuration */
#define CONFIG_SERIAL3		/* use SERIAL 3 */
#define CONFIG_DEFAULT_CONSOLE	"console=ttySAC1,115200n8\0"

#define CONFIG_SYS_PROMPT	"Peach-Pi # "
#define CONFIG_IDENT_STRING	" for Peach-Pi"

#define CONFIG_VIDEO_PARADE

/* Display */
#define CONFIG_LCD
#ifdef CONFIG_LCD
#define CONFIG_EXYNOS_FB
#define CONFIG_EXYNOS_DP
#define LCD_BPP			LCD_COLOR16
#endif

#define CONFIG_POWER_TPS65090_EC
#define CONFIG_CROS_EC_SPI		/* Support CROS_EC over SPI */
#define CONFIG_DM_CROS_EC

#define CONFIG_USB_XHCI
#define CONFIG_USB_XHCI_EXYNOS

/* DRAM Memory Banks */
#define CONFIG_NR_DRAM_BANKS	7
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */

#endif	/* __CONFIG_PEACH_PI_H */
