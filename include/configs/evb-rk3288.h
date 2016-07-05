/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS
#include <configs/rk3288_common.h>

#define CONFIG_SPL_MMC_SUPPORT

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 1
/* SPL @ 32k for ~36k
 * ENV @ 96k
 * u-boot @ 128K
 */
#define CONFIG_ENV_OFFSET (96 * 1024)

#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_CONSOLE_SCROLL_LINES		10

#endif
