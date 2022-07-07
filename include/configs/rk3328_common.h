/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_RK3328_COMMON_H
#define __CONFIG_RK3328_COMMON_H

#include "rockchip-common.h"

#define CONFIG_IRAM_BASE		0xff090000

/* FAT sd card locations. */
#define CONFIG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xff000000

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00500000\0" \
	"pxefile_addr_r=0x00600000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"kernel_addr_r=0x02080000\0" \
	"ramdisk_addr_r=0x06000000\0"

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	BOOTENV

#endif
