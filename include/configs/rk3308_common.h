/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_RK3308_COMMON_H
#define __CONFIG_RK3308_COMMON_H

#include "rockchip-common.h"

#define CFG_IRAM_BASE			0xfff80000

#define CFG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xff000000

#define ENV_MEM_LAYOUT_SETTINGS		\
	"scriptaddr=0x00500000\0"	\
	"script_offset_f=0xffe000\0"	\
	"script_size_f=0x2000\0"	\
	"pxefile_addr_r=0x00600000\0"	\
	"fdt_addr_r=0x01e00000\0"	\
	"fdtoverlay_addr_r=0x01f00000\0"	\
	"kernel_addr_r=0x02080000\0"	\
	"ramdisk_addr_r=0x06000000\0"	\
	"kernel_comp_addr_r=0x08000000\0"	\
	"kernel_comp_size=0x2000000\0"

#define CFG_EXTRA_ENV_SETTINGS		\
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"	\
	"partitions=" PARTS_DEFAULT	\
	ENV_MEM_LAYOUT_SETTINGS		\
	ROCKCHIP_DEVICE_SETTINGS	\
	"boot_targets=" BOOT_TARGETS "\0"

#endif /* __CONFIG_RK3308_COMMON_H */
