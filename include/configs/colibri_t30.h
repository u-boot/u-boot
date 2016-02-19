/*
 * Copyright (c) 2013-2015 Stefan Agner
 *
 * Configuration settings for the Toradex Colibri T30 modules.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra30-common.h"

#define CONFIG_ARCH_MISC_INIT

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"Toradex Colibri T30"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_COLIBRI_T30

/* I2C */
#define CONFIG_SYS_I2C_TEGRA
#define CONFIG_CMD_I2C

/* SD/MMC support */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2

/* USB host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX

/* General networking support */
#define CONFIG_CMD_DHCP
#define CONFIG_IP_DEFRAG
#define CONFIG_TFTP_BLOCKSIZE		16384
#define CONFIG_TFTP_TSIZE

/* Miscellaneous commands */
#define CONFIG_FAT_WRITE

/* Increase console I/O buffer size */
#undef CONFIG_SYS_CBSIZE
#define CONFIG_SYS_CBSIZE		1024

/* Increase arguments buffer size */
#undef CONFIG_SYS_BARGSIZE
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/* Increase print buffer size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* Increase maximum number of arguments */
#undef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS		32

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"

#endif /* __CONFIG_H */
