/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra124-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"Google/NVIDIA Nyan-big"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

/* Environment in eMMC, at the end of 2nd "boot sector" */

/* Align LCD to 1MB boundary */
#define CONFIG_LCD_ALIGNMENT	MMU_SECTION_SIZE

/* SPI */
#define CONFIG_SPI_FLASH_SIZE          (4 << 20)

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
