/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>

#include "tegra30-common.h"

/* Enable fdt support for Cardhu. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra30-cardhu
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define V_PROMPT		"Tegra30 (Cardhu) # "
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Cardhu"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_CARDHU

#define CONFIG_BOARD_EARLY_INIT_F

/* I2C */
#define CONFIG_TEGRA_I2C
#define CONFIG_SYS_I2C_INIT_BOARD
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_MAX_I2C_BUS		TEGRA_I2C_NUM_CONTROLLERS
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_CMD_I2C

#define CONFIG_ENV_IS_NOWHERE

/* SPI */
#define CONFIG_TEGRA_SLINK
#define CONFIG_TEGRA_SLINK_CTRLS       6
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SF_DEFAULT_MODE         SPI_MODE_0
#define CONFIG_SF_DEFAULT_SPEED        24000000
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_SIZE          (4 << 20)

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
