/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __PUMA_RK3399_H
#define __PUMA_RK3399_H

#include <configs/rk3399_common.h>

/*
 * SPL @ 32kB for ~130kB
 * ENV @ 240KB for 8kB
 * FIT payload (ATF, U-Boot, FDT) @ 256kB
 */
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET (240 * 1024)

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 1

#define SDRAM_BANK_SIZE			(2UL << 30)

#endif
