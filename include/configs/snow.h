/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 Snow board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SNOW_H
#define __CONFIG_SNOW_H

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SPI_FLASH
#define CONFIG_ENV_SPI_BASE	0x12D30000
#define FLASH_SIZE		(0x4 << 20)
#define CONFIG_ENV_OFFSET	(FLASH_SIZE - CONFIG_BL2_SIZE)
#define CONFIG_SPI_BOOTING

#include <configs/exynos5250-common.h>
#include <configs/exynos5-dt-common.h>


#define CONFIG_CROS_EC_I2C		/* Support CROS_EC over I2C */
#define CONFIG_POWER_TPS65090_I2C

#define CONFIG_BOARD_COMMON
#define CONFIG_ARCH_EARLY_INIT_R

#define CONFIG_USB_XHCI
#define CONFIG_USB_XHCI_EXYNOS

#define CONFIG_SYS_PROMPT		"snow # "
#define CONFIG_IDENT_STRING		" for snow"
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#endif	/* __CONFIG_SNOW_H */
