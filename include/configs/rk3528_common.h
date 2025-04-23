/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright Contributors to the U-Boot project. */

#ifndef __CONFIG_RK3528_COMMON_H
#define __CONFIG_RK3528_COMMON_H

#define CFG_CPUID_OFFSET		0xa

#include "rockchip-common.h"

#define CFG_IRAM_BASE			0xfe480000

#define CFG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xfc000000

#ifndef ROCKCHIP_DEVICE_SETTINGS
#define ROCKCHIP_DEVICE_SETTINGS
#endif

#define ENV_MEM_LAYOUT_SETTINGS			\
	"scriptaddr=0x00c00000\0"		\
	"script_offset_f=0xffe000\0"		\
	"script_size_f=0x2000\0"		\
	"pxefile_addr_r=0x00e00000\0"		\
	"kernel_addr_r=0x02000000\0"		\
	"kernel_comp_addr_r=0x0a000000\0"	\
	"fdt_addr_r=0x12000000\0"		\
	"fdtoverlay_addr_r=0x12100000\0"	\
	"ramdisk_addr_r=0x12180000\0"		\
	"kernel_comp_size=0x8000000\0"

#define CFG_EXTRA_ENV_SETTINGS			\
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"	\
	ENV_MEM_LAYOUT_SETTINGS			\
	ROCKCHIP_DEVICE_SETTINGS		\
	"boot_targets=" BOOT_TARGETS "\0"

#endif /* __CONFIG_RK3528_COMMON_H */
