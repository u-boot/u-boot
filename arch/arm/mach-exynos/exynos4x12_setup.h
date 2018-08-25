/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * refer to exynox4_setup.h
 * Machine Specific Values for EXYNOS4412 based board
 *
 * Copyright (C) 2018 Wang Xinlu <wangkartx@gmail.com>
 */

#ifndef _EXYNOX4x12_SETUP_H
#define _EXYNOX4x12_SETUP_H

#include <config.h>

#ifdef CONFIG_CLK_800_330_165
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_200_200
#define DRAM_CLK_200
#endif
#ifdef CONFIG_CLK_1000_330_165
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_400_200
#define DRAM_CLK_400
#endif

#endif
