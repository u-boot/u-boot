/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Lucas Stach
 *
 * Configuration settings for the Toradex Colibri T20 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra20-common.h"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
