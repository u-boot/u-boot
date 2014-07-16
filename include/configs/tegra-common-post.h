/*
 * (C) Copyright 2010-2012
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TEGRA_COMMON_POST_H
#define __TEGRA_COMMON_POST_H

#ifdef CONFIG_BOOTCOMMAND

#define BOOTCMDS_COMMON ""

#else

#ifdef CONFIG_CMD_MMC
#define BOOTCMDS_MMC \
	"mmc_boot=" \
		"setenv devtype mmc; " \
		"if mmc dev ${devnum}; then " \
			"run scan_boot; " \
		"fi\0" \
	"bootcmd_mmc0=setenv devnum 0; run mmc_boot;\0" \
	"bootcmd_mmc1=setenv devnum 1; run mmc_boot;\0"
#define BOOT_TARGETS_MMC "mmc1 mmc0"
#else
#define BOOTCMDS_MMC ""
#define BOOT_TARGETS_MMC ""
#endif

#ifdef CONFIG_CMD_USB
#define BOOTCMD_INIT_USB "run usb_init; "
#define BOOTCMDS_USB \
	"usb_init=" \
		"if ${usb_need_init}; then " \
			"set usb_need_init false; " \
			"usb start 0; " \
		"fi\0" \
	\
	"usb_boot=" \
		"setenv devtype usb; " \
		BOOTCMD_INIT_USB \
		"if usb dev ${devnum}; then " \
			"run scan_boot; " \
		"fi\0" \
	\
	"bootcmd_usb0=setenv devnum 0; run usb_boot;\0"
#define BOOT_TARGETS_USB "usb0"
#else
#define BOOTCMD_INIT_USB ""
#define BOOTCMDS_USB ""
#define BOOT_TARGETS_USB ""
#endif

#ifdef CONFIG_CMD_DHCP
#define BOOTCMDS_DHCP \
	"bootcmd_dhcp=" \
		BOOTCMD_INIT_USB \
		"if dhcp ${scriptaddr} boot.scr.uimg; then "\
			"source ${scriptaddr}; " \
		"fi\0"
#define BOOT_TARGETS_DHCP "dhcp"
#else
#define BOOTCMDS_DHCP ""
#define BOOT_TARGETS_DHCP ""
#endif

#if defined(CONFIG_CMD_DHCP) && defined(CONFIG_CMD_PXE)
#define BOOTCMDS_PXE \
	"bootcmd_pxe=" \
		BOOTCMD_INIT_USB \
		"dhcp; " \
		"if pxe get; then " \
			"pxe boot; " \
		"fi\0"
#define BOOT_TARGETS_PXE "pxe"
#else
#define BOOTCMDS_PXE ""
#define BOOT_TARGETS_PXE ""
#endif

#define BOOTCMDS_COMMON \
	"rootpart=1\0" \
	\
	"do_script_boot="                                                 \
		"load ${devtype} ${devnum}:${rootpart} "                  \
			"${scriptaddr} ${prefix}${script}; "              \
		"source ${scriptaddr}\0"                                  \
	\
	"script_boot="                                                    \
		"for script in ${boot_scripts}; do "                      \
			"if test -e ${devtype} ${devnum}:${rootpart} "    \
					"${prefix}${script}; then "       \
				"echo Found U-Boot script "               \
					"${prefix}${script}; "            \
				"run do_script_boot; "                    \
				"echo SCRIPT FAILED: continuing...; "     \
			"fi; "                                            \
		"done\0"                                                  \
	\
	"do_sysboot_boot="                                                \
		"sysboot ${devtype} ${devnum}:${rootpart} any "           \
			"${scriptaddr} ${prefix}extlinux/extlinux.conf\0" \
	\
	"sysboot_boot="                                                   \
		"if test -e ${devtype} ${devnum}:${rootpart} "            \
				"${prefix}extlinux/extlinux.conf; then "  \
			"echo Found ${prefix}extlinux/extlinux.conf; "    \
			"run do_sysboot_boot; "                           \
			"echo SCRIPT FAILED: continuing...; "             \
		"fi\0"                                                    \
	\
	"scan_boot="                                                      \
		"echo Scanning ${devtype} ${devnum}...; "                 \
		"for prefix in ${boot_prefixes}; do "                     \
			"run sysboot_boot; "                              \
			"run script_boot; "                               \
		"done\0"                                                  \
	\
	"boot_targets=" \
		BOOT_TARGETS_MMC " " \
		BOOT_TARGETS_USB " " \
		BOOT_TARGETS_PXE " " \
		BOOT_TARGETS_DHCP " " \
		"\0" \
	\
	"boot_prefixes=/ /boot/\0" \
	\
	"boot_scripts=boot.scr.uimg boot.scr\0" \
	\
	BOOTCMDS_MMC \
	BOOTCMDS_USB \
	BOOTCMDS_DHCP \
	BOOTCMDS_PXE

#define CONFIG_BOOTCOMMAND \
	"set usb_need_init; " \
	"for target in ${boot_targets}; do run bootcmd_${target}; done"

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
	BOOTCMDS_COMMON \
	BOARD_EXTRA_ENV_SETTINGS

#if defined(CONFIG_TEGRA20_SFLASH) || defined(CONFIG_TEGRA20_SLINK) || defined(CONFIG_TEGRA114_SPI)
#define CONFIG_FDT_SPI
#endif

/* overrides for SPL build here */
#ifdef CONFIG_SPL_BUILD

#define CONFIG_SKIP_LOWLEVEL_INIT

/* remove devicetree support */
#ifdef CONFIG_OF_CONTROL
#undef CONFIG_OF_CONTROL
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
