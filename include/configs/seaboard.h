/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* LP0 suspend / resume */
#define CONFIG_TEGRA_LP0
#define CONFIG_TEGRA_PMU
#define CONFIG_TPS6586X_POWER
#define CONFIG_TEGRA_CLOCK_SCALING

#include "tegra20-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Seaboard"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* Environment in eMMC, at the end of 2nd "boot sector" */

/* NAND support */

/* Max number of NAND devices */
#define CONFIG_SYS_MAX_NAND_DEVICE	1

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
