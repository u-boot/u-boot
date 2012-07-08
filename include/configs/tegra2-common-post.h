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

#ifndef __TEGRA2_COMMON_POST_H
#define __TEGRA2_COMMON_POST_H

#ifdef CONFIG_BOOTCOMMAND

#define BOOTCMDS_COMMON ""

#else

#ifdef CONFIG_CMD_EXT2
#define BOOTCMD_FS_EXT2 "ext2 "
#else
#define BOOTCMD_FS_EXT2 ""
#endif

#ifdef CONFIG_CMD_FAT
#define BOOTCMD_FS_FAT "fat"
#else
#define BOOTCMD_FS_FAT ""
#endif

#ifdef CONFIG_CMD_MMC
#define BOOTCMDS_MMC \
	"mmc_boot=" \
		"setenv devtype mmc; " \
		"if mmc dev ${devnum}; then " \
			"run script_boot; " \
		"fi\0" \
	"mmc0_boot=setenv devnum 0; run mmc_boot;\0" \
	"mmc1_boot=setenv devnum 1; run mmc_boot;\0" \
	"bootcmd_mmc=run mmc1_boot; run mmc0_boot\0"
#define BOOTCMD_MMC "run bootcmd_mmc; "
#else
#define BOOTCMDS_MMC ""
#define BOOTCMD_MMC ""
#endif

#ifdef CONFIG_CMD_USB
#define BOOTCMDS_USB \
	"usb_boot=" \
		"setenv devtype usb; " \
		"if usb dev ${devnum}; then " \
			"run script_boot; " \
		"fi\0" \
	"usb0_boot=setenv devnum 0; run usb_boot;\0" \
	"bootcmd_usb=run usb0_boot\0"
#define BOOTCMD_USB "run bootcmd_usb; "
#define BOOTCMD_INIT_USB "usb start 0; "
#else
#define BOOTCMDS_USB ""
#define BOOTCMD_USB ""
#define BOOTCMD_INIT_USB ""
#endif

#ifdef CONFIG_CMD_DHCP
#define BOOTCMDS_DHCP \
	"bootcmd_dhcp=" \
		"if dhcp ${scriptaddr} boot.scr.uimg; then "\
			"source ${scriptaddr}; " \
		"fi\0"
#define BOOTCMD_DHCP "run bootcmd_dhcp; "
#else
#define BOOTCMDS_DHCP ""
#define BOOTCMD_DHCP ""
#endif

#define BOOTCMDS_COMMON \
	"scriptaddr=0x400000\0" \
	"rootpart=1\0" \
	"script_boot="													\
		"for fs in " BOOTCMD_FS_EXT2 BOOTCMD_FS_FAT "; do "							\
		    "for prefix in / /boot/; do "									\
			"for script in boot.scr.uimg boot.scr; do "							\
			    "echo Scanning ${devtype} ${devnum}:${rootpart} ${fs} ${prefix}${script} ...; "		\
			    "if ${fs}load ${devtype} ${devnum}:${rootpart} ${scriptaddr} ${prefix}${script}; then "	\
				"echo ${script} found! Executing ...;"							\
				"source ${scriptaddr};"									\
			    "fi; "											\
			"done; "											\
		    "done; "												\
		"done;\0"												\
	BOOTCMDS_MMC \
	BOOTCMDS_USB \
	BOOTCMDS_DHCP

#define CONFIG_BOOTCOMMAND BOOTCMD_INIT_USB BOOTCMD_USB BOOTCMD_MMC BOOTCMD_DHCP

#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	TEGRA2_DEVICE_SETTINGS \
	BOOTCMDS_COMMON

#endif /* __TEGRA2_COMMON_POST_H */
