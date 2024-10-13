/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014-2016 Marcel Ziswiler
 *
 * Configuration settings for the Toradex Apalis T30 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra30-common.h"

/*
 * Board-specific serial config
 *
 * Apalis UART1: NVIDIA UARTA
 * Apalis UART2: NVIDIA UARTD
 * Apalis UART3: NVIDIA UARTB
 * Apalis UART4: NVIDIA UARTC
 */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
