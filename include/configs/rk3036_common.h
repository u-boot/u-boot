/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */
#ifndef __CONFIG_RK3036_COMMON_H
#define __CONFIG_RK3036_COMMON_H

#include <asm/arch-rockchip/hardware.h>
#include "rockchip-common.h"

#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_ROCKCHIP_STIMER_BASE	0x200440a0
#define COUNTER_FREQUENCY		24000000
#define CONFIG_SYS_ARCH_TIMER
#define CONFIG_SYS_HZ_CLOCK		24000000

#define CONFIG_SYS_INIT_SP_ADDR		0x60100000
#define CONFIG_SYS_LOAD_ADDR		0x60800800
#define CONFIG_SPL_STACK		0x10081fff

#define CONFIG_ROCKCHIP_MAX_INIT_SIZE	(4 << 10)
#define CONFIG_ROCKCHIP_CHIP_TAG	"RK30"

#define CONFIG_SYS_SDRAM_BASE		0x60000000
#define SDRAM_BANK_SIZE			(512UL << 20UL)
#define SDRAM_MAX_SIZE                  (CONFIG_NR_DRAM_BANKS * SDRAM_BANK_SIZE)

#ifndef CONFIG_SPL_BUILD

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x60000000\0" \
	"pxefile_addr_r=0x60100000\0" \
	"fdt_addr_r=0x61f00000\0" \
	"kernel_addr_r=0x62000000\0" \
	"ramdisk_addr_r=0x64000000\0"

#include <config_distro_bootcmd.h>

/* Linux fails to load the fdt if it's loaded above 512M on a evb-rk3036 board,
 * so limit the fdt reallocation to that */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_high=0x7fffffff\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV
#endif

#endif
