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
/* A variant of Exynos5420 (Exynos5 Family) */
#define CONFIG_EXYNOS5800

#include <configs/exynos5-common.h>

#define CONFIG_ARCH_EARLY_INIT_R

#define MACH_TYPE_SMDK5420	8002
#define CONFIG_MACH_TYPE	MACH_TYPE_SMDK5420

#define CONFIG_VAR_SIZE_SPL

#ifdef CONFIG_VAR_SIZE_SPL
#define CONFIG_SPL_TEXT_BASE		0x02024410
#else
#define CONFIG_SPL_TEXT_BASE		0x02024400
#endif
#define CONFIG_IRAM_TOP			0x02074000

#define CONFIG_SPL_MAX_FOOTPRINT	(30 * 1024)

#define CONFIG_DEVICE_TREE_LIST "exynos5800-peach-pi"	\
				"exynos5420-peach-pit exynos5420-smdk5420"

#define CONFIG_MAX_I2C_NUM	11

#define CONFIG_BOARD_REV_GPIO_COUNT	2

#endif	/* __CONFIG_EXYNOS5420_H */
