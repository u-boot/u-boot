/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Pumpkin board
 *
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Fabien Parent <fparent@baylibre.com
 */

#ifndef __PUMPKIN_H
#define __PUMPKIN_H

#include <linux/sizes.h>

#define CONFIG_ENV_SIZE			SZ_4K
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MALLOC_LEN		SZ_4M

#define CONFIG_CPU_ARMV8
#define COUNTER_FREQUENCY		13000000

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_COM1		0x11005000
#define CONFIG_SYS_NS16550_CLK		26000000

#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + SZ_2M - \
						 GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

/* Environment settings */
#include <config_distro_bootcmd.h>

#define MMCBOOT \
	"mmcdev=0\0" \
	"kernel_partition=2\0" \
	"rootfs_partition=3\0" \
	"mmc_discover_partition=" \
		"part start mmc ${mmcdev} ${kernel_partition} kernel_part_addr;" \
		"part size mmc ${mmcdev} ${kernel_partition} kernel_part_size;\0" \
	"mmcboot=" \
		"mmc dev ${mmcdev};" \
		"run mmc_discover_partition;" \
		"mmc read ${kerneladdr} ${kernel_part_addr} ${kernel_part_size};" \
		"setenv bootargs ${bootargs} root=/dev/mmcblk${mmcdev}p${rootfs_partition} rootwait; " \
		"bootm ${kerneladdr}; \0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kerneladdr=0x4A000000\0" \
	MMCBOOT \
	"bootcmd=run mmcboot;\0"

#endif
