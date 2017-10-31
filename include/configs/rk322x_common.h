/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef __CONFIG_RK322X_COMMON_H
#define __CONFIG_RK322X_COMMON_H

#include <asm/arch/hardware.h>
#include "rockchip-common.h"

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_MALLOC_LEN		(32 << 20)
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/*  64M */

#define CONFIG_SYS_TIMER_RATE		(24 * 1000 * 1000)
#define CONFIG_SYS_TIMER_BASE		0x110c00a0 /* TIMER5 */
#define CONFIG_SYS_TIMER_COUNTER	(CONFIG_SYS_TIMER_BASE + 8)

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_TEXT_BASE		0x60000000
#define CONFIG_SYS_INIT_SP_ADDR		0x60100000
#define CONFIG_SYS_LOAD_ADDR		0x60800800
#define CONFIG_SPL_STACK		0x10088000
#define CONFIG_SPL_TEXT_BASE		0x10081004

#define CONFIG_ROCKCHIP_MAX_INIT_SIZE	(28 << 10)
#define CONFIG_ROCKCHIP_CHIP_TAG	"RK32"

/* MMC/SD IP block */
#define CONFIG_BOUNCE_BUFFER

#define CONFIG_SYS_SDRAM_BASE		0x60000000
#define CONFIG_NR_DRAM_BANKS		2
#define SDRAM_BANK_SIZE			(512UL << 20UL)
#define SDRAM_MAX_SIZE			0x80000000

#ifndef CONFIG_SPL_BUILD
/* usb otg */

/* usb mass storage */
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_CMD_USB_MASS_STORAGE

/* usb host */
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
	"fdt_high=0x7fffffff\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV
#endif

#define CONFIG_PREBOOT

#endif
