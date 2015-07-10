/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG/GOOGLE PEACH-PIT board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_PEACH_PIT_H
#define __CONFIG_PEACH_PIT_H

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BASE	0x12D30000
#define FLASH_SIZE		(0x4 << 20)
#define CONFIG_ENV_OFFSET	(FLASH_SIZE - CONFIG_BL2_SIZE)
#define CONFIG_SPI_BOOTING

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x22000000\0" \
	"fdt_addr_r=0x23000000\0" \
	"ramdisk_addr_r=0x23300000\0" \
	"scriptaddr=0x30000000\0" \
	"pxefile_addr_r=0x31000000\0"

#include <configs/exynos5420-common.h>
#include <configs/exynos5-dt-common.h>

#define CONFIG_BOARD_COMMON

#define CONFIG_SYS_SDRAM_BASE	0x20000000
#define CONFIG_SYS_TEXT_BASE	0x23E00000
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_IRAM_TOP - 0x800)

/* select serial console configuration */
#define CONFIG_SERIAL3		/* use SERIAL 3 */
#define CONFIG_DEFAULT_CONSOLE	"console=ttySAC1,115200n8\0"

#define CONFIG_SYS_PROMPT	"Peach-Pit # "
#define CONFIG_IDENT_STRING	" for Peach-Pit"

#define CONFIG_VIDEO_PARADE

/* Display */
#define CONFIG_LCD
#ifdef CONFIG_LCD
#define CONFIG_EXYNOS_FB
#define CONFIG_EXYNOS_DP
#define LCD_BPP			LCD_COLOR16
#endif

#define CONFIG_POWER_TPS65090_EC

#define CONFIG_USB_XHCI
#define CONFIG_USB_XHCI_EXYNOS

/* DRAM Memory Banks */
#define CONFIG_NR_DRAM_BANKS	4
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */

#endif	/* __CONFIG_PEACH_PIT_H */
