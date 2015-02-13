/*
 * Copyright (C) 2014 Google, Inc
 *
 * Configuration settings for generic Exynos 5 board
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_EXYNOS5_DT_COMMON_H
#define __CONFIG_EXYNOS5_DT_COMMON_H

/* Console configuration */
#undef EXYNOS_DEVICE_SETTINGS
#define EXYNOS_DEVICE_SETTINGS \
		"stdin=serial,cros-ec-keyb\0" \
		"stdout=serial,lcd\0" \
		"stderr=serial,lcd\0"

#include "exynos5-common.h"

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_TPS65090

/* Enable keyboard */
#define CONFIG_KEYBOARD

#endif
