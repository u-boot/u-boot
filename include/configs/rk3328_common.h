/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_RK3328_COMMON_H
#define __CONFIG_RK3328_COMMON_H

#include "rockchip-common.h"

#define CONFIG_SYS_MALLOC_LEN		(32 << 20)
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_SYS_NS16550_MEM32

#define CONFIG_SYS_TEXT_BASE		0x00200000
#define CONFIG_SYS_INIT_SP_ADDR		0x00300000
#define CONFIG_SYS_LOAD_ADDR		0x00800800

#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* 64M */

/* MMC/SD IP block */
#define CONFIG_BOUNCE_BUFFER

#define CONFIG_SUPPORT_VFAT
#define CONFIG_FS_EXT4

/* RAW SD card / eMMC locations. */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(128 << 10)

/* FAT sd card locations. */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_NR_DRAM_BANKS		1
#define SDRAM_MAX_SIZE			0xff000000

#define CONFIG_SPI_FLASH
#define CONFIG_SPI
#define CONFIG_SF_DEFAULT_SPEED 20000000

#ifndef CONFIG_SPL_BUILD

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00500000\0" \
	"pxefile_addr_r=0x00600000\0" \
	"fdt_addr_r=0x01f00000\0" \
	"kernel_addr_r=0x02000000\0" \
	"ramdisk_addr_r=0x04000000\0"

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"partitions=" PARTS_DEFAULT \
	BOOTENV

#endif

/* rockchip ohci host driver */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1

#endif
