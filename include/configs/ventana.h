/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>
#include "tegra2-common.h"

/* Enable fdt support for Ventana. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra2-seaboard
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define TEGRA2_SYSMEM		"mem=384M@0M nvmem=128M@384M mem=512M@512M"
#define V_PROMPT		"Tegra2 (Ventana) # "
#define CONFIG_TEGRA2_BOARD_STRING	"NVIDIA Ventana"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA2_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_VENTANA
#define CONFIG_SYS_BOARD_ODMDATA	0x300d8011 /* lp1, 1GB */

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
