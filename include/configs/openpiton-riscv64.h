/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2021 Tianrui Wei
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 *   Tianrui Wei <tianrui-wei@outlook.com>
 */

#ifndef __OPENPITON_RISCV64_CONFIG_H
#define __OPENPITON_RISCV64_CONFIG_H

#include <linux/sizes.h>

/* Environment options */
#define CONFIG_SYS_SDRAM_BASE 0x80000000
#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_SDRAM_BASE + SZ_32M)
#define CONFIG_SYS_BOOTM_LEN        SZ_256M

#ifdef CONFIG_SPL
#define CONFIG_SPL_MAX_SIZE     0x00100000
#define CONFIG_SPL_BSS_START_ADDR   0x82000000
#define CONFIG_SPL_BSS_MAX_SIZE     0x00100000
#define CONFIG_SYS_SPL_MALLOC_START (CONFIG_SPL_BSS_START_ADDR + \
		CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE  0x0100000
#define CONFIG_SPL_STACK    (0x80000000 + 0x04000000 - \
		GENERATED_GBL_DATA_SIZE)

#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME "boot/fw_payload.bin"
#define CONFIG_SPL_GD_ADDR 0x85000000
#endif

/* ---------------------------------------------------------------------
 * Board boot configuration
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_addr_r=0x86000000\0" \
	"kernel_addr_r=0x80200000\0" \
	"image=boot/Image\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0"

#endif/* __CONFIG_H */
