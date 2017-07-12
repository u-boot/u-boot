/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIGS_LION_RK3368_H
#define __CONFIGS_LION_RK3368_H

#include <configs/rk3368_common.h>

#define CONFIG_SYS_MMC_ENV_DEV		0
#define KERNEL_LOAD_ADDR		0x280000
#define DTB_LOAD_ADDR			0x5600000
#define INITRD_LOAD_ADDR		0x5bf0000
#define CONFIG_ENV_SIZE			0x2000

#endif
