/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_PX30_COMMON_H
#define __CONFIG_PX30_COMMON_H

#include "rockchip-common.h"

#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_SYS_NS16550_MEM32

/* FIXME: ff020000 is pmu_mem (10k), while ff0e0000 is regular int_mem */
#define CONFIG_IRAM_BASE		0xff020000

#define CONFIG_SYS_INIT_SP_ADDR		0x00400000
#define CONFIG_SPL_STACK		0x00400000
#define CONFIG_SPL_MAX_SIZE		0x20000
#define CONFIG_SPL_BSS_START_ADDR	0x4000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x4000
#define CONFIG_SYS_BOOTM_LEN		(64 << 20)	/* 64M */

#define GICD_BASE			0xff131000
#define GICC_BASE			0xff132000

#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* 64M */

#define CONFIG_SYS_SDRAM_BASE		0
#define SDRAM_MAX_SIZE			0xff000000
#define SDRAM_BANK_SIZE			(2UL << 30)

#ifndef CONFIG_SPL_BUILD

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00500000\0" \
	"pxefile_addr_r=0x00600000\0" \
	"fdt_addr_r=0x08300000\0" \
	"kernel_addr_r=0x00280000\0" \
	"kernel_addr_c=0x03e80000\0" \
	"ramdisk_addr_r=0x0a200000\0"

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	ROCKCHIP_DEVICE_SETTINGS \
	BOOTENV

#endif

#endif
