/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG/GOOGLE PEACH-PIT board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_PEACH_PIT_H
#define __CONFIG_PEACH_PIT_H

#include <configs/exynos5420-common.h>
#include <configs/exynos5-dt-common.h>


/* select serial console configuration */
#define CONFIG_SERIAL3		/* use SERIAL 3 */

#define CONFIG_SYS_PROMPT	"Peach # "
#define CONFIG_IDENT_STRING	" for Peach"

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

#endif	/* __CONFIG_PEACH_PIT_H */
