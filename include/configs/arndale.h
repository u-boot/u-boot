/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG Arndale board.
 */

#ifndef __CONFIG_ARNDALE_H
#define __CONFIG_ARNDALE_H

#define EXYNOS_FDTFILE_SETTING \
	"fdtfile=exynos5250-arndale.dtb\0"

#include "exynos5250-common.h"
#include <configs/exynos5-common.h>

/* MMC SPL */
#define CONFIG_EXYNOS_SPL

/* Miscellaneous configurable options */
#define CONFIG_IRAM_STACK	0x02050000

#define CONFIG_SYS_INIT_SP_ADDR	CONFIG_IRAM_STACK

#define CONFIG_S5P_PA_SYSRAM	0x02020000
#define CONFIG_SMP_PEN_ADDR	CONFIG_S5P_PA_SYSRAM

/* The PERIPHBASE in the CBAR register is wrong on the Arndale, so override it */
#define CONFIG_ARM_GIC_BASE_ADDRESS	0x10480000

#endif	/* __CONFIG_H */
