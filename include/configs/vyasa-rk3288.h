/*
 * Copyright (C) 2017 Amarula Solutions
 *
 * Configuration settings for Amarula Vyasa RK3288.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS
#include <configs/rk3288_common.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \

#define CONFIG_SYS_MMC_ENV_DEV 1
#undef CONFIG_CMD_USB_MASS_STORAGE

#endif
