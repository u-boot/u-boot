/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDK5250 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SMDK_H
#define __CONFIG_SMDK_H

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SPI_FLASH
#define CONFIG_ENV_SPI_BASE	0x12D30000
#define FLASH_SIZE		(0x4 << 20)
#define CONFIG_ENV_OFFSET	(FLASH_SIZE - CONFIG_BL2_SIZE)
#define CONFIG_SPI_BOOTING

#include <configs/exynos5250-common.h>

/* PMIC */
#define CONFIG_POWER_MAX77686

#define CONFIG_BOARD_COMMON
#define CONFIG_ARCH_EARLY_INIT_R

#define CONFIG_USB_XHCI
#define CONFIG_USB_XHCI_EXYNOS

#define CONFIG_SYS_PROMPT		"SMDK5250 # "
#define CONFIG_IDENT_STRING		" for SMDK5250"

/* Miscellaneous configurable options */
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#endif	/* __CONFIG_SMDK_H */
