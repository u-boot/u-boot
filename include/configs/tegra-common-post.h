/*
 * (C) Copyright 2010-2012
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TEGRA_COMMON_POST_H
#define __TEGRA_COMMON_POST_H

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

#ifdef CONFIG_TEGRA_KEYBOARD
#define STDIN_KBD_KBC ",tegra-kbc"
#else
#define STDIN_KBD_KBC ""
#endif

#ifdef CONFIG_USB_KEYBOARD
#define STDIN_KBD_USB ",usbkbd"
#define CONFIG_SYS_USB_EVENT_POLL
#define CONFIG_PREBOOT			"usb start"
#else
#define STDIN_KBD_USB ""
#endif

#ifdef CONFIG_VIDEO_TEGRA
#define STDOUT_LCD ",lcd"
#else
#define STDOUT_LCD ""
#endif

#define TEGRA_DEVICE_SETTINGS \
	"stdin=serial" STDIN_KBD_KBC STDIN_KBD_USB "\0" \
	"stdout=serial" STDOUT_LCD "\0" \
	"stderr=serial" STDOUT_LCD "\0" \
	""

#ifndef BOARD_EXTRA_ENV_SETTINGS
#define BOARD_EXTRA_ENV_SETTINGS
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	TEGRA_DEVICE_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"fdt_high=ffffffff\0" \
	"initrd_high=ffffffff\0" \
	BOOTENV \
	BOARD_EXTRA_ENV_SETTINGS

#if defined(CONFIG_TEGRA20_SFLASH) || defined(CONFIG_TEGRA20_SLINK) || defined(CONFIG_TEGRA114_SPI)
#define CONFIG_FDT_SPI
#endif

/* overrides for SPL build here */
#ifdef CONFIG_SPL_BUILD

#define CONFIG_SKIP_LOWLEVEL_INIT

/* remove devicetree support */
#ifdef CONFIG_OF_CONTROL
#endif

/* remove I2C support */
#ifdef CONFIG_SYS_I2C_TEGRA
#undef CONFIG_SYS_I2C_TEGRA
#endif
#ifdef CONFIG_CMD_I2C
#undef CONFIG_CMD_I2C
#endif

/* remove MMC support */
#ifdef CONFIG_MMC
#undef CONFIG_MMC
#endif
#ifdef CONFIG_GENERIC_MMC
#undef CONFIG_GENERIC_MMC
#endif
#ifdef CONFIG_TEGRA_MMC
#undef CONFIG_TEGRA_MMC
#endif
#ifdef CONFIG_CMD_MMC
#undef CONFIG_CMD_MMC
#endif

/* remove partitions/filesystems */
#ifdef CONFIG_DOS_PARTITION
#undef CONFIG_DOS_PARTITION
#endif
#ifdef CONFIG_EFI_PARTITION
#undef CONFIG_EFI_PARTITION
#endif
#ifdef CONFIG_CMD_FS_GENERIC
#undef CONFIG_CMD_FS_GENERIC
#endif
#ifdef CONFIG_CMD_EXT4
#undef CONFIG_CMD_EXT4
#endif
#ifdef CONFIG_CMD_EXT2
#undef CONFIG_CMD_EXT2
#endif
#ifdef CONFIG_CMD_FAT
#undef CONFIG_CMD_FAT
#endif
#ifdef CONFIG_FS_EXT4
#undef CONFIG_FS_EXT4
#endif
#ifdef CONFIG_FS_FAT
#undef CONFIG_FS_FAT
#endif

/* remove USB */
#ifdef CONFIG_USB_EHCI
#undef CONFIG_USB_EHCI
#endif
#ifdef CONFIG_USB_EHCI_TEGRA
#undef CONFIG_USB_EHCI_TEGRA
#endif
#ifdef CONFIG_USB_STORAGE
#undef CONFIG_USB_STORAGE
#endif
#ifdef CONFIG_CMD_USB
#undef CONFIG_CMD_USB
#endif

/* remove part command support */
#ifdef CONFIG_PARTITION_UUIDS
#undef CONFIG_PARTITION_UUIDS
#endif

#ifdef CONFIG_CMD_PART
#undef CONFIG_CMD_PART
#endif

#endif /* CONFIG_SPL_BUILD */

#endif /* __TEGRA_COMMON_POST_H */
