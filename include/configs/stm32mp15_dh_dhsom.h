/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020 Marek Vasut <marex@denx.de>
 *
 * Configuration settings for the DH STM32MP15x SoMs
 */

#ifndef __CONFIG_STM32MP15_DH_DHSOM_H__
#define __CONFIG_STM32MP15_DH_DHSOM_H__

/* PHY needs a longer autoneg timeout */
#define PHY_ANEG_TIMEOUT		20000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"dfu_alt_info_ram=u-boot.itb ram "				\
			__stringify(CONFIG_SPL_LOAD_FIT_ADDRESS)	\
			" 0x800000\0"
#endif

#define STM32MP_BOARD_EXTRA_ENV \
	"usb_pgood_delay=1000\0" \
	"update_sf=" /* Erase SPI NOR and install U-Boot from SD */	\
		"setexpr loadaddr1 ${loadaddr} + 0x1000000 && "		\
		"load mmc 0:4 ${loadaddr1} /boot/u-boot-spl.stm32 && "	\
		"env set filesize1 ${filesize} && "			\
		"load mmc 0:4 ${loadaddr} /boot/u-boot.itb && "		\
		"sf probe && sf erase 0 0x200000 && "			\
		"sf update ${loadaddr1} 0 ${filesize1} && "		\
		"sf update ${loadaddr1} 0x40000 ${filesize1} && "	\
		"sf update ${loadaddr} 0x80000 ${filesize} && "		\
		"env set filesize1 && env set loadaddr1\0"

#include <configs/stm32mp15_common.h>

#endif
