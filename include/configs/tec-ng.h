/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Avionic Design GmbH <www.avionic-design.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra30-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"Avionic Design Tamonten™ NG Evaluation Carrier"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* Environment in eMMC, at the end of 2nd "boot sector" */

/* SPI */
#define CONFIG_TEGRA_SLINK_CTRLS       6
#define CONFIG_SPI_FLASH_SIZE          (4 << 20)

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
