/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Common configuration settings for the SAMSUNG EXYNOS boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __EXYNOS_COMMON_H
#define __EXYNOS_COMMON_H

/* High Level Configuration Options */
#define CONFIG_SAMSUNG			/* in a SAMSUNG core */
#define CONFIG_S5P			/* S5P Family */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <linux/sizes.h>

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_USE_ARCH_MEMCPY
#define CONFIG_USE_ARCH_MEMSET

/* Keep L2 Cache Disabled */

/* input clock of PLL: 24MHz input clock */
#define CONFIG_SYS_CLK_FREQ		24000000
#define CONFIG_TIMER_CLK_FREQ		CONFIG_SYS_CLK_FREQ

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_ENV_OVERWRITE

/* Size of malloc() pool before and after relocation */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (80 << 20))

/* select serial console configuration */
#define CONFIG_BAUDRATE			115200

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_S5P_SDHCI
#define CONFIG_SDHCI
#define CONFIG_DWMMC
#define CONFIG_EXYNOS_DWMMC
#define CONFIG_BOUNCE_BUFFER

/* PWM */
#define CONFIG_PWM

/* Command definition*/
#define CONFIG_FAT_WRITE

#define CONFIG_CMD_PART
#define CONFIG_PARTITION_UUIDS

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		1024	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#include <config_distro_defaults.h>

#endif	/* __CONFIG_H */
