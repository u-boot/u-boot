/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (c) Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *
 * Based on jetson-tk1.h which is:
 * (C) Copyright 2013-2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra124-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"CEI tk1-som"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* SPI */
#define CONFIG_SPI_FLASH_SIZE		(4 << 20)

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
