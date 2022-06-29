/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2012
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __TEGRA_COMMON_POST_H
#define __TEGRA_COMMON_POST_H

#define CONFIG_SYS_NONCACHED_MEMORY	(1 << 20)	/* 1 MiB */

#if CONFIG_IS_ENABLED(CMD_USB)
# define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
# define BOOT_TARGET_USB(func)
#endif

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	BOOT_TARGET_USB(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif
#include <config_distro_bootcmd.h>

#ifdef CONFIG_TEGRA_KEYBOARD
#define STDIN_KBD_KBC ",tegra-kbc"
#else
#define STDIN_KBD_KBC ""
#endif

#ifdef CONFIG_USB_KEYBOARD
#define STDIN_KBD_USB ",usbkbd"
#else
#define STDIN_KBD_USB ""
#endif

#ifdef CONFIG_LCD
#define STDOUT_LCD ",lcd"
#else
#define STDOUT_LCD ""
#endif

#ifdef CONFIG_DM_VIDEO
#define STDOUT_VIDEO ",vidconsole"
#else
#define STDOUT_VIDEO ""
#endif

#ifdef CONFIG_CROS_EC_KEYB
#define STDOUT_CROS_EC	",cros-ec-keyb"
#else
#define STDOUT_CROS_EC	""
#endif

#define TEGRA_DEVICE_SETTINGS \
	"stdin=serial" STDIN_KBD_KBC STDIN_KBD_USB STDOUT_CROS_EC "\0" \
	"stdout=serial" STDOUT_LCD STDOUT_VIDEO "\0" \
	"stderr=serial" STDOUT_LCD STDOUT_VIDEO "\0" \
	""

#ifndef BOARD_EXTRA_ENV_SETTINGS
#define BOARD_EXTRA_ENV_SETTINGS
#endif

#ifdef CONFIG_ARM64
#define FDT_HIGH "ffffffffffffffff"
#define INITRD_HIGH "ffffffffffffffff"
#else
#define FDT_HIGH "ffffffff"
#define INITRD_HIGH "ffffffff"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	TEGRA_DEVICE_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"fdt_high=" FDT_HIGH "\0" \
	"initrd_high=" INITRD_HIGH "\0" \
	BOOTENV \
	BOARD_EXTRA_ENV_SETTINGS

#if defined(CONFIG_TEGRA20_SFLASH) || defined(CONFIG_TEGRA20_SLINK) || defined(CONFIG_TEGRA114_SPI)
#define CONFIG_TEGRA_SPI
#endif

#endif /* __TEGRA_COMMON_POST_H */
