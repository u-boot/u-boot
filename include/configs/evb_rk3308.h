/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2018 Rockchip Electronics Co., Ltd
 */

#ifndef __EVB_RK3308_H
#define __EVB_RK3308_H

#include <configs/rk3308_common.h>

#define CONFIG_SUPPORT_EMMC_RPMB
#define CONFIG_SYS_MMC_ENV_DEV 0

#define ROCKCHIP_DEVICE_SETTINGS \
			"stdout=serial,vidconsole\0" \
			"stderr=serial,vidconsole\0"
#undef CONFIG_CONSOLE_SCROLL_LINES
#define CONFIG_CONSOLE_SCROLL_LINES            10

#endif
