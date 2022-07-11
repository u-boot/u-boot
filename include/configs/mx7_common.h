/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7.
 */

#ifndef __MX7_COMMON_H
#define __MX7_COMMON_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

/* Timer settings */
#define CONFIG_MXC_GPT_HCLK
#define CONFIG_SC_TIMER_CLK 8000000 /* 8Mhz */

/* Enable iomux-lpsr support */
#define CONFIG_IOMUX_LPSR

/* Miscellaneous configurable options */

/* UART */

/* MMC */

/*
 * If we have defined the OPTEE ram size and not OPTEE it means that we were
 * launched by OPTEE, because of that we shall skip all the low level
 * initialization since it was already done by ATF or OPTEE
 */

#endif
