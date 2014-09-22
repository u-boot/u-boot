/*
 * (C) Copyright 2013
 * Avionic Design GmbH <www.avionic-design.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra30-common.h"

/* High-level configuration options */
#define V_PROMPT			"Tegra30 (TEC-NG) # "
#define CONFIG_TEGRA_BOARD_STRING	"Avionic Design Tamonten™ NG Evaluation Carrier"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

#define CONFIG_BOARD_EARLY_INIT_F

/* I2C */
#define CONFIG_SYS_I2C_TEGRA
#define CONFIG_SYS_I2C_INIT_BOARD
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_MAX_I2C_BUS		TEGRA_I2C_NUM_CONTROLLERS
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2

/* SPI */
#define CONFIG_TEGRA20_SLINK
#define CONFIG_TEGRA_SLINK_CTRLS       6
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SF_DEFAULT_MODE         SPI_MODE_0
#define CONFIG_SF_DEFAULT_SPEED        24000000
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_SIZE          (4 << 20)

/* USB Host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX

/* General networking support */
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP

/* Tag support */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
/* support the new (FDT-based) image format */
#define CONFIG_FIT

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
