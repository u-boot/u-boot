/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Topic Embedded Products
 *
 * Configuration for Zynq Evaluation and Development Board - Miami
 * See zynq-common.h for Zynq common configs
 */

#ifndef __CONFIG_TOPIC_MIAMI_H
#define __CONFIG_TOPIC_MIAMI_H

#ifndef CONFIG_XPL_BUILD

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#if defined(CONFIG_ZYNQ_QSPI)
# define BOOT_TARGET_DEVICES_QSPI(func)	func(QSPI, qspi, na)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#ifdef CONFIG_CMD_UBIFS
# define BOOT_TARGET_DEVICES_UBIFS(func) func(UBIFS, ubifs, 0, qspi-rootfs, qspi-rootfs)
#else
# define BOOT_TARGET_DEVICES_UBIFS(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=sf probe && " \
		      "sf read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
		      "echo QSPI: Trying to boot script at ${scriptaddr} && " \
		      "source ${scriptaddr}; echo QSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_UBIFS(func) \
	BOOT_TARGET_DEVICES_QSPI(func)

#include <config_distro_bootcmd.h>

#endif /* CONFIG_XPL_BUILD */

/* Default environment */
#ifndef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x3000000\0"	\
	"script_offset_f=0xf0000\0"	\
	"script_size_f=0x10000\0"	\
	"fdt_addr_r=0x1f00000\0"	\
	"pxefile_addr_r=0x2000000\0"	\
	"kernel_addr_r=0x2000000\0"	\
	"ramdisk_addr_r=0x3100000\0"	\
	BOOTENV
#endif

#include "zynq-common.h"

/* Detect RAM size */
#define CFG_SYS_SDRAM_BASE 0
#define CFG_SYS_SDRAM_SIZE 0x40000000

#endif /* __CONFIG_TOPIC_MIAMI_H */
