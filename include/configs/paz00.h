/*
 * Copyright (c) 2010-2012 NVIDIA CORPORATION.  All rights reserved.
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
#include "tegra20-common.h"

/* Enable fdt support for Paz00. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra20-paz00
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define V_PROMPT		"Tegra20 (Paz00) MOD # "
#define CONFIG_TEGRA_BOARD_STRING	"Compal Paz00"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_PAZ00

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

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET ((1024 * 1024) - CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV 0
#define CONFIG_SYS_MMC_ENV_PART 2

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
