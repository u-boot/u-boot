/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013-2016 Stefan Agner
 *
 * Configuration settings for the Toradex Colibri T30 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra30-common.h"

/* High-level configuration options */

/*
 * Board-specific serial config
 *
 * Colibri UART-A: NVIDIA UARTA
 * Colibri UART-B: NVIDIA UARTD
 * Colibri UART-C: NVIDIA UARTB
 */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
