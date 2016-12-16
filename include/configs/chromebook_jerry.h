/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS \
		"stdin=serial,cros-ec-keyb\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"

#include <configs/rk3288_common.h>

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPI_FLASH_GIGADEVICE

#define CONFIG_CMD_SF_TEST

#define CONFIG_KEYBOARD

#define CONFIG_SYS_WHITE_ON_BLACK

#endif
