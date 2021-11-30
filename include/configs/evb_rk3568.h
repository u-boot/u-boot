/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#ifndef __EVB_RK3568_H
#define __EVB_RK3568_H

#include <configs/rk3568_common.h>

#define CONFIG_SUPPORT_EMMC_RPMB

#define ROCKCHIP_DEVICE_SETTINGS \
			"stdout=serial,vidconsole\0" \
			"stderr=serial,vidconsole\0"

#endif
