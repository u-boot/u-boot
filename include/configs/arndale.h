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

/* Miscellaneous configurable options */

#define CONFIG_SMP_PEN_ADDR	0x02020000

/* The PERIPHBASE in the CBAR register is wrong on the Arndale, so override it */
#define CONFIG_ARM_GIC_BASE_ADDRESS	0x10480000

#endif	/* __CONFIG_H */
