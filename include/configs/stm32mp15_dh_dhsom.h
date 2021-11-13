/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020 Marek Vasut <marex@denx.de>
 *
 * Configuration settings for the DH STM32MP15x SoMs
 */

#ifndef __CONFIG_STM32MP15_DH_DHSOM_H__
#define __CONFIG_STM32MP15_DH_DHSOM_H__

#define STM32MP_BOARD_EXTRA_ENV \
	"usb_pgood_delay=1000\0"

#include <configs/stm32mp15_common.h>

#define CONFIG_SPL_TARGET		"u-boot.itb"

#endif
