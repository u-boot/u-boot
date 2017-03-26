/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS
#include <configs/rk3188_common.h>

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 0

#ifdef CONFIG_ROCKCHIP_SPL_BACK_TO_BROM
/* SPL @ 32k for 34k
 * u-boot directly after @ 68k for 400k or so
 * ENV @ 992k
 */
#define CONFIG_ENV_OFFSET ((1024-32) * 1024)
#else
/* SPL @ 32k for ~36k
 * ENV @ 96k
 * u-boot @ 128K
 */
#define CONFIG_ENV_OFFSET (96 * 1024)
#endif

#endif
