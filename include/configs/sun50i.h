/*
 * Configuration settings for the Allwinner A64 (sun50i) CPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * A64 specific configuration
 */

#ifdef CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_SUNXI
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

#define CONFIG_SUNXI_USB_PHYS	1

#define COUNTER_FREQUENCY	CONFIG_TIMER_CLK_FREQ
#define GICD_BASE		0x1c81000
#define GICC_BASE		0x1c82000

/*
 * Include common sunxi configuration where most the settings are
 */
#include <configs/sunxi-common.h>

#endif /* __CONFIG_H */
