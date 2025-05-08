/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2025
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 *
 *  Generic device header which can be used with SYS_CONFIG_NAME
 *  for any Tegra device (T20, T30, T114, T124, T186 or T210).
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#ifdef CONFIG_TEGRA20
#include "tegra20-common.h"
#elif CONFIG_TEGRA30
#include "tegra30-common.h"
#elif CONFIG_TEGRA114
#include "tegra114-common.h"
#elif CONFIG_TEGRA124
#include "tegra124-common.h"
#elif CONFIG_TEGRA186
#include "tegra186-common.h"
#elif CONFIG_TEGRA210
#include "tegra210-common.h"
#endif

#ifdef CONFIG_TEGRA_PRAM
  #define CFG_PRAM CONFIG_TEGRA_PRAM_SIZE
#endif

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
