/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDK5250 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SMDK_H
#define __CONFIG_SMDK_H

#include <configs/exynos5250-common.h>
#include <configs/exynos5-dt-common.h>
#include <configs/exynos5-common.h>

#undef CONFIG_LCD
#undef CONFIG_EXYNOS_FB
#undef CONFIG_EXYNOS_DP
#undef CONFIG_KEYBOARD

#define CONFIG_BOARD_COMMON

#define CONFIG_IDENT_STRING		" for SMDK5250"
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#endif	/* __CONFIG_SMDK_H */
