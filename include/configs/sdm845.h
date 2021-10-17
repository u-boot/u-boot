/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for boards, based on Qualcomm SDM845 chip
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#ifndef __CONFIGS_SDM845_H
#define __CONFIGS_SDM845_H

#include <linux/sizes.h>
#include <asm/arch/sysmap-sdm845.h>

#define CONFIG_SYS_LOAD_ADDR	0x80000000
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 230400, 460800, 921600 }

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY	19000000

#define EXTRA_ENV_SETTINGS \

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80000000\0"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN   (CONFIG_ENV_SIZE + SZ_8M)
#define CONFIG_SYS_BOOTM_LEN	SZ_64M

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	512
#define CONFIG_SYS_MAXARGS	64

#endif
