/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT8518 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#ifndef __MT8518_H
#define __MT8518_H

#include <linux/sizes.h>

#define CONFIG_SYS_NONCACHED_MEMORY		SZ_1M

#define CONFIG_CPU_ARMV8

#define COUNTER_FREQUENCY			13000000

/* DRAM definition */
#define CONFIG_SYS_SDRAM_BASE			0x40000000
#define CONFIG_SYS_SDRAM_SIZE			0x20000000

#define CONFIG_SYS_LOAD_ADDR			0x41000000
#define CONFIG_LOADADDR				CONFIG_SYS_LOAD_ADDR

#define CONFIG_SYS_MALLOC_LEN			SZ_32M
#define CONFIG_SYS_BOOTM_LEN			SZ_64M

/* Uboot definition */
#define CONFIG_SYS_INIT_SP_ADDR			(CONFIG_SYS_TEXT_BASE + \
						SZ_2M - \
						GENERATED_GBL_DATA_SIZE)

/* ENV Setting */
#if defined(CONFIG_MMC_MTK)
#define CONFIG_SYS_MMC_ENV_DEV			0
#define CONFIG_ENV_OVERWRITE

/* MMC offset in block unit,and block size is 0x200 */
#define ENV_BOOT_READ_IMAGE \
	"boot_rd_img=mmc dev 0" \
	";mmc read ${loadaddr} 0x27400 0x4000" \
	";iminfo ${loadaddr}\0"
#endif

/* Console configuration */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

#define ENV_BOOT_CMD \
	"mtk_boot=run boot_rd_img;bootm;\0"

#define ENV_FASTBOOT \
	"serial#=1234567890ABCDEF\0" \
	"board=mt8518\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x6c000000\0" \
	ENV_DEVICE_SETTINGS \
	ENV_BOOT_READ_IMAGE \
	ENV_FASTBOOT \
	ENV_BOOT_CMD \
	"bootcmd=run mtk_boot;\0" \

#endif
