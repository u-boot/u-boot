/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef __CONFIG_RK3288_COMMON_H
#define __CONFIG_RK3288_COMMON_H

#include <asm/arch-rockchip/hardware.h>
#include "rockchip-common.h"

#define CONFIG_SYS_BOOTM_LEN (16 << 20) /* 16MB */

#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY
#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_ROCKCHIP_STIMER_BASE	0xff810020
#define COUNTER_FREQUENCY		24000000
#define CONFIG_SYS_ARCH_TIMER
#define CONFIG_SYS_HZ_CLOCK		24000000

#ifdef CONFIG_SPL_ROCKCHIP_BACK_TO_BROM
/* Bootrom will load u-boot binary to 0x0 once return from SPL */
#endif
#define CONFIG_SYS_INIT_SP_ADDR		0x00100000
#define CONFIG_SYS_LOAD_ADDR		0x00800800
#define CONFIG_SPL_STACK		0xff718000

#define CONFIG_IRAM_BASE		0xff700000

/* RAW SD card / eMMC locations. */
#ifdef CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#endif

/* FAT sd card locations. */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

#define CONFIG_SYS_SDRAM_BASE		0
#define SDRAM_BANK_SIZE			(2UL << 30)
#define SDRAM_MAX_SIZE			0xfe000000

#define CONFIG_SYS_MONITOR_LEN (600 * 1024)

#ifndef CONFIG_SPL_BUILD

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00000000\0" \
	"pxefile_addr_r=0x00100000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"kernel_addr_r=0x02000000\0" \
	"ramdisk_addr_r=0x04000000\0"

#include <config_distro_bootcmd.h>

/* Linux fails to load the fdt if it's loaded above 256M on a Rock 2 board, so
 * limit the fdt reallocation to that */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x0fffffff\0" \
	"initrd_high=0x0fffffff\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	ENV_MEM_LAYOUT_SETTINGS \
	ROCKCHIP_DEVICE_SETTINGS \
	BOOTENV
#endif

#endif
