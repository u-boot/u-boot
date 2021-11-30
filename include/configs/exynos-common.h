/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Common configuration settings for the SAMSUNG EXYNOS boards.
 */

#ifndef __EXYNOS_COMMON_H
#define __EXYNOS_COMMON_H

/* High Level Configuration Options */
#define CONFIG_SAMSUNG			/* in a SAMSUNG core */
#define CONFIG_S5P			/* S5P Family */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <linux/sizes.h>
#include <linux/stringify.h>

/* Keep L2 Cache Disabled */

/* input clock of PLL: 24MHz input clock */
#define CONFIG_SYS_CLK_FREQ		24000000
#define COUNTER_FREQUENCY		CONFIG_SYS_CLK_FREQ

/* select serial console configuration */

/* PWM */
#define CONFIG_PWM

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		1024	/* Print Buffer Size */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#endif	/* __CONFIG_H */
