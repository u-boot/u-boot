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

/* Enable fdt support for Harmony. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra20-harmony
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define V_PROMPT		"Tegra20 (Harmony) # "
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Harmony"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD

/* UARTD: keyboard satellite board UART, default */
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE
#ifdef CONFIG_TEGRA_ENABLE_UARTA
/* UARTA: debug board UART */
#define CONFIG_SYS_NS16550_COM2		NV_PA_APB_UARTA_BASE
#endif

#define CONFIG_MACH_TYPE		MACH_TYPE_HARMONY

#define CONFIG_BOARD_EARLY_INIT_F

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/* NAND support */
#define CONFIG_CMD_NAND
#define CONFIG_TEGRA_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE	NV_PA_NAND_BASE

/* Environment in NAND (which is 512M), aligned to start of last sector */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET	(SZ_512M - SZ_128K) /* 128K sector size */

/* USB Host support */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 3
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_ULPI
#define CONFIG_USB_ULPI_VIEWPORT
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_USB_ETHER_ASIX

/* General networking support */
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
