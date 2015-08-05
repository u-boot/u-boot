/*
 * (C) Copyright 2014 Chen-Yu Tsai <wens@csie.org>
 *
 * Configuration settings for the Allwinner A23 (sun8i) CPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * A23 specific configuration
 */

#ifdef CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_SUNXI
#define CONFIG_USB_MAX_CONTROLLER_COUNT	1
#endif

#define CONFIG_SUNXI_USB_PHYS	2

#define CONFIG_ARMV7_PSCI		1
#if defined(CONFIG_MACH_SUN8I_A23)
#define CONFIG_ARMV7_PSCI_NR_CPUS	2
#elif defined(CONFIG_MACH_SUN8I_A33)
#define CONFIG_ARMV7_PSCI_NR_CPUS	4
#else
#error Unsupported sun8i variant
#endif
#define CONFIG_TIMER_CLK_FREQ		24000000

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
