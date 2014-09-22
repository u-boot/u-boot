/*
 *  Copyright (C) 2012 Lucas Stach
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra20-common.h"

/* High-level configuration options */
#define V_PROMPT                   "Tegra20 (Colibri) # "
#define CONFIG_TEGRA_BOARD_STRING  "Toradex Colibri T20 on Iris"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_TEGRA_UARTA_SDIO1
#define CONFIG_SYS_NS16550_COM1    NV_PA_APB_UARTA_BASE

#define CONFIG_BOARD_EARLY_INIT_F

/* SD/MMC support */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

/* USB host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_ULPI
#define CONFIG_USB_ULPI_VIEWPORT
#define CONFIG_USB_STORAGE
#define CONFIG_USB_MAX_CONTROLLER_COUNT 3
#define CONFIG_CMD_USB

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING

/* NAND support */
#define CONFIG_CMD_NAND
#define CONFIG_TEGRA_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE     1

/* Environment in NAND, 64K is a bit excessive but erase block is 512K anyway */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET              (SZ_2M)
#undef  CONFIG_ENV_SIZE /* undef size from tegra20-common.h */
#define CONFIG_ENV_SIZE                (SZ_64K)

/* Debug commands */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_CACHE

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
