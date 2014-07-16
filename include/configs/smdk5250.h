/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDK5250 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SMDK_H
#define __CONFIG_SMDK_H

#include <configs/exynos5250-dt.h>

#undef CONFIG_DEFAULT_DEVICE_TREE
#define CONFIG_DEFAULT_DEVICE_TREE	exynos5250-smdk5250

/* Enable FIT support and comparison */
#define CONFIG_FIT
#define CONFIG_FIT_BEST_MATCH

#endif	/* __CONFIG_SMDK_H */
