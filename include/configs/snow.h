/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 Snow board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SNOW_H
#define __CONFIG_SNOW_H

#include <configs/exynos5250-common.h>
#include <configs/exynos5-dt-common.h>
#include <configs/exynos5-common.h>

#define CONFIG_BOARD_COMMON

#define CONFIG_IDENT_STRING		" for snow"
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#endif	/* __CONFIG_SNOW_H */
