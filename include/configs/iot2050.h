/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for IOT2050
 * Copyright (c) Siemens AG, 2018-2021
 *
 * Authors:
 *   Le Jin <le.jin@siemens.com>
 *   Jan Kiszka <jan.kiszka@siemens.com>
 */

#ifndef __CONFIG_IOT2050_H
#define __CONFIG_IOT2050_H

#include <linux/sizes.h>

/* SPL Loader Configuration */

/* U-Boot general configuration */
#define EXTRA_ENV_IOT2050_BOARD_SETTINGS				\
	"usb_pgood_delay=900\0"

#if CONFIG_IS_ENABLED(CMD_USB)
# define BOOT_TARGET_USB(func) \
	func(USB, usb, 0) \
	func(USB, usb, 1) \
	func(USB, usb, 2)
#else
# define BOOT_TARGET_USB(func)
#endif

/*
 * This defines all MMC devices, even if the basic variant has no mmc1.
 * The non-supported device will be removed from the boot targets during
 * runtime, when that board was detected.
 */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	BOOT_TARGET_USB(func)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS					\
	DEFAULT_LINUX_BOOT_ENV						\
	BOOTENV								\
	EXTRA_ENV_IOT2050_BOARD_SETTINGS

#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_IOT2050_H */
