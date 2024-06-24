/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Toradex, Inc.
 *
 * Configuration settings for the Toradex Apalis TK1 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra124-common.h"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define FDT_MODULE			"apalis-v1.2"
#define FDT_MODULE_V1_0			"apalis"

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
