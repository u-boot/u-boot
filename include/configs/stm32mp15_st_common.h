/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2021, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STMicroelectonics STM32MP15x boards
 */

#ifndef __CONFIG_STM32MP15_ST_COMMON_H__
#define __CONFIG_STM32MP15_ST_COMMON_H__

#include <configs/stm32mp15_common.h>

#ifdef CONFIG_EXTRA_ENV_SETTINGS
/*
 * default bootcmd for stm32mp1 STMicroelectronics boards:
 * for serial/usb: execute the stm32prog command
 * for mmc boot (eMMC, SD card), distro boot on the same mmc device
 * for nand or spi-nand boot, distro boot with ubifs on UBI partition
 * for nor boot, distro boot on SD card = mmc0 ONLY !
 */
#define ST_STM32MP1_BOOTCMD "bootcmd_stm32mp=" \
	"echo \"Boot over ${boot_device}${boot_instance}!\";" \
	"if test ${boot_device} = serial || test ${boot_device} = usb;" \
	"then stm32prog ${boot_device} ${boot_instance}; " \
	"else " \
		"run env_check;" \
		"if test ${boot_device} = mmc;" \
		"then env set boot_targets \"mmc${boot_instance}\"; fi;" \
		"if test ${boot_device} = nand ||" \
		  " test ${boot_device} = spi-nand ;" \
		"then env set boot_targets ubifs0; fi;" \
		"if test ${boot_device} = nor;" \
		"then env set boot_targets mmc0; fi;" \
		"run distro_bootcmd;" \
	"fi;\0"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	STM32MP_MEM_LAYOUT \
	ST_STM32MP1_BOOTCMD \
	STM32MP_PARTS_DEFAULT \
	BOOTENV \
	STM32MP_EXTRA

#endif
#endif
