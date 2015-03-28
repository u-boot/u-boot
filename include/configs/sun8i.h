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

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
