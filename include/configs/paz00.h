/*
 * Copyright (c) 2010,2011, NVIDIA CORPORATION.  All rights reserved.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>
#include "tegra2-common.h"

/* High-level configuration options */
#define TEGRA2_SYSMEM		"mem=512M@0M"
#define V_PROMPT		"Tegra2 (Paz00) MOD # "
#define CONFIG_TEGRA2_BOARD_STRING	"Compal Paz00"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA2_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_PAZ00
#define CONFIG_SYS_BOARD_ODMDATA	0x800c0085 /* lp1, 512MB */

#define CONFIG_BOARD_EARLY_INIT_F

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA2_MMC
#define CONFIG_CMD_MMC

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/* Environment not stored */
#define CONFIG_ENV_IS_NOWHERE
#endif /* __CONFIG_H */
