/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Configuration settings for the SAMSUNG Arndale board.
 */

#ifndef __CONFIG_ARNDALE_H
#define __CONFIG_ARNDALE_H

#define EXYNOS_FDTFILE_SETTING \
	"fdtfile=exynos5250-arndale.dtb\0"

#include "exynos5250-common.h"
#include <configs/exynos5-common.h>

/* SD/MMC configuration */
#define CONFIG_SUPPORT_EMMC_BOOT

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* MMC SPL */
#define CONFIG_EXYNOS_SPL

/* Miscellaneous configurable options */
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET	(CONFIG_BL2_OFFSET + CONFIG_BL2_SIZE)

#define CONFIG_IRAM_STACK	0x02050000

#define CONFIG_SYS_INIT_SP_ADDR	CONFIG_IRAM_STACK

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C

#define CONFIG_PREBOOT

#define CONFIG_S5P_PA_SYSRAM	0x02020000
#define CONFIG_SMP_PEN_ADDR	CONFIG_S5P_PA_SYSRAM

/* The PERIPHBASE in the CBAR register is wrong on the Arndale, so override it */
#define CONFIG_ARM_GIC_BASE_ADDRESS	0x10480000

/* Power */
#define CONFIG_POWER
#define CONFIG_POWER_I2C

#endif	/* __CONFIG_H */
