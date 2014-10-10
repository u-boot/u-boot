/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5420 SoC
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_EXYNOS5420_H
#define __CONFIG_EXYNOS5420_H

#define CONFIG_EXYNOS5420

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SPI_FLASH
#define CONFIG_ENV_SPI_BASE	0x12D30000
#define FLASH_SIZE		(0x4 << 20)
#define CONFIG_ENV_OFFSET	(FLASH_SIZE - CONFIG_BL2_SIZE)
#define CONFIG_SPI_BOOTING

#include <configs/exynos5-common.h>

#define CONFIG_ARCH_EARLY_INIT_R

#define MACH_TYPE_SMDK5420	8002
#define CONFIG_MACH_TYPE	MACH_TYPE_SMDK5420

#define CONFIG_VAR_SIZE_SPL

#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_TEXT_BASE		0x23E00000
#ifdef CONFIG_VAR_SIZE_SPL
#define CONFIG_SPL_TEXT_BASE		0x02024410
#else
#define CONFIG_SPL_TEXT_BASE		0x02024400
#endif
#define CONFIG_IRAM_TOP			0x02074000

#define CONFIG_SPL_MAX_FOOTPRINT	(30 * 1024)

#define CONFIG_DEVICE_TREE_LIST "exynos5420-peach-pit exynos5420-smdk5420"

#define CONFIG_MAX_I2C_NUM	11

#define CONFIG_BOARD_REV_GPIO_COUNT	2

#define CONFIG_BOOTCOMMAND	"mmc read 20007000 451 2000; bootm 20007000"

#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	2

/*
 * Put the initial stack pointer 1KB below this to allow room for the
 * SPL marker. This value is arbitrary, but gd_t is placed starting here.
 */
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_IRAM_TOP - 0x800)

/* DRAM Memory Banks */
#define CONFIG_NR_DRAM_BANKS	7
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */

/* Miscellaneous configurable options */
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#endif	/* __CONFIG_EXYNOS5420_H */
