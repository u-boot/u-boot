/*
 * (C) Copyright 2010-2012
 * NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __TEGRA_COMMON_POST_H
#define __TEGRA_COMMON_POST_H

#ifdef CONFIG_BOOTCOMMAND

#define BOOTCMDS_COMMON ""

#else

#ifdef CONFIG_CMD_EXT2
#define BOOT_FSTYPE_EXT2 "ext2 "
#else
#define BOOT_FSTYPE_EXT2 ""
#endif

#ifdef CONFIG_CMD_FAT
#define BOOT_FSTYPE_FAT "fat"
#else
#define BOOT_FSTYPE_FAT ""
#endif

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

#define BOOTCMDS_COMMON \
	"scriptaddr=0x400000\0" \
	\
	"rootpart=1\0" \
	\
	"script_boot="                                                    \
		"if ${fs}load ${devtype} ${devnum}:${rootpart} "          \
				"${scriptaddr} ${prefix}${script}; then " \
			"echo ${script} found! Executing ...;"            \
			"source ${scriptaddr};"                           \
		"fi;\0"                                                   \
	\
	"scan_boot="                                                      \
		"echo Scanning ${devtype} ${devnum}...; "                 \
		"for fs in ${boot_fstypes}; do "                          \
			"for prefix in ${boot_prefixes}; do "             \
				"for script in ${boot_scripts}; do "      \
					"run script_boot; "               \
				"done; "                                  \
			"done; "                                          \
		"done;\0"                                                 \
	\
	"boot_targets=" \
		BOOT_TARGETS_MMC " " \
		BOOT_TARGETS_USB " " \
		BOOT_TARGETS_DHCP " " \
		"\0" \
	\
	"boot_fstypes=" \
		BOOT_FSTYPE_EXT2 " " \
		BOOT_FSTYPE_FAT " " \
		"\0" \
	\
	"boot_prefixes=/ /boot/\0" \
	\
	"boot_scripts=boot.scr.uimg boot.scr\0" \
	\
	BOOTCMDS_MMC \
	BOOTCMDS_USB \
	BOOTCMDS_DHCP

#define CONFIG_BOOTCOMMAND \
	"for target in ${boot_targets}; do run bootcmd_${target}; done"

#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	TEGRA_DEVICE_SETTINGS \
	"fdt_load=0x01000000\0" \
	"fdt_high=01100000\0" \
	BOOTCMDS_COMMON

/* overrides for SPL build here */
#ifdef CONFIG_SPL_BUILD

/* remove devicetree support */
#ifdef CONFIG_OF_CONTROL
#undef CONFIG_OF_CONTROL
#endif

/* remove SERIAL_MULTI */
#ifdef CONFIG_SERIAL_MULTI
#undef CONFIG_SERIAL_MULTI
#endif

/* remove I2C support */
#ifdef CONFIG_TEGRA_I2C
#undef CONFIG_TEGRA_I2C
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
#ifdef CONFIG_CMD_EXT2
#undef CONFIG_CMD_EXT2
#endif
#ifdef CONFIG_CMD_FAT
#undef CONFIG_CMD_FAT
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

#endif /* CONFIG_SPL_BUILD */

#endif /* __TEGRA_COMMON_POST_H */
