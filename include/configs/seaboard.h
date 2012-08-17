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

/* Enable fdt support for Seaboard. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra2-seaboard
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define TEGRA2_SYSMEM		"mem=384M@0M nvmem=128M@384M mem=512M@512M"
#define V_PROMPT		"Tegra2 (SeaBoard) # "
#define CONFIG_TEGRA2_BOARD_STRING	"NVIDIA Seaboard"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA2_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* On Seaboard: GPIO_PI3 = Port I = 8, bit = 3 */
#define CONFIG_UART_DISABLE_GPIO	GPIO_PI3

#define CONFIG_MACH_TYPE		MACH_TYPE_SEABOARD
#define CONFIG_SYS_BOARD_ODMDATA	0x300d8011 /* lp1, 1GB */

#define CONFIG_BOARD_EARLY_INIT_F

/* SPI */
#define CONFIG_TEGRA2_SPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_SIZE		(4 << 20)

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
#define CONFIG_TEGRA2_MMC
#define CONFIG_CMD_MMC

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/* Environment in SPI */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_MAX_HZ		48000000
#define CONFIG_ENV_SPI_MODE		SPI_MODE_0

#define CONFIG_ENV_SECT_SIZE    CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET       (CONFIG_SPI_FLASH_SIZE - CONFIG_ENV_SECT_SIZE)

/* USB Host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

#endif /* __CONFIG_H */
