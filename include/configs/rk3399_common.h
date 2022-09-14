/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_RK3399_COMMON_H
#define __CONFIG_RK3399_COMMON_H

#include "rockchip-common.h"

#define CONFIG_IRAM_BASE		0xff8c0000

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_TPL_BOOTROM_SUPPORT)
#else
/*  BSS setup */
#endif

/* MMC/SD IP block */
#define CONFIG_ROCKCHIP_SDHCI_MAX_FREQ	200000000

/* RAW SD card / eMMC locations. */

/* FAT sd card locations. */
#define CONFIG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xf8000000

#define ROCKPI_4B_IDBLOADER_IMAGE_GUID \
	EFI_GUID(0x02f4d760, 0xcfd5, 0x43bd, 0x8e, 0x2d, \
		 0xa4, 0x2a, 0xcb, 0x33, 0xc6, 0x60)

#define ROCKPI_4B_UBOOT_IMAGE_GUID \
	EFI_GUID(0x4ce292da, 0x1dd8, 0x428d, 0xa1, 0xc2, \
		 0x77, 0x74, 0x3e, 0xf8, 0xb9, 0x6e)

#define ROCKPI_4C_IDBLOADER_IMAGE_GUID \
	EFI_GUID(0xfd68510c, 0x12d3, 0x4f0a, 0xb8, 0xd3, \
		 0xd8, 0x79, 0xe1, 0xd3, 0xa5, 0x40)

#define ROCKPI_4C_UBOOT_IMAGE_GUID \
	EFI_GUID(0xb81fb4ae, 0xe4f3, 0x471b, 0x99, 0xb4, \
		 0x0b, 0x3d, 0xa5, 0x49, 0xce, 0x13)

#ifndef CONFIG_SPL_BUILD

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00500000\0" \
	"script_offset_f=0xffe000\0" \
	"script_size_f=0x2000\0" \
	"pxefile_addr_r=0x00600000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"fdtoverlay_addr_r=0x02000000\0" \
	"kernel_addr_r=0x02080000\0" \
	"ramdisk_addr_r=0x06000000\0" \
	"kernel_comp_addr_r=0x08000000\0" \
	"kernel_comp_size=0x2000000\0"

#ifndef ROCKCHIP_DEVICE_SETTINGS
#define ROCKCHIP_DEVICE_SETTINGS
#endif

#include <config_distro_bootcmd.h>
#include <environment/distro/sf.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	ROCKCHIP_DEVICE_SETTINGS \
	BOOTENV \
	BOOTENV_SF \
	"altbootcmd=" \
		"setenv boot_syslinux_conf extlinux/extlinux-rollback.conf;" \
		"run distro_bootcmd\0"

#endif

#endif
