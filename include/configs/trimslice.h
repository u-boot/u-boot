/*
 *  (C) Copyright 2010-2012
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
#include "tegra20-common.h"

/* Enable fdt support for TrimSlice. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra20-trimslice
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define V_PROMPT		"Tegra20 (TrimSlice) # "
#define CONFIG_TEGRA_BOARD_STRING	"Compulab Trimslice"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_TEGRA_UARTA_GPU
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_TRIMSLICE

#define CONFIG_BOARD_EARLY_INIT_F

/* SPI */
#define CONFIG_TEGRA_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF

/* I2C */
#define CONFIG_TEGRA_I2C
#define CONFIG_SYS_I2C_INIT_BOARD
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_MAX_I2C_BUS		4
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_CMD_I2C

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/* Environment in SPI */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_MAX_HZ		48000000
#define CONFIG_ENV_SPI_MODE		SPI_MODE_0
#define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET		(512 * 1024)

/* USB Host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX

/* General networking support */
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
