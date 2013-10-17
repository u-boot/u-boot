/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated.
 * Steve Sakoman  <steve@sakoman.com>
 *
 * Configuration settings for the TI OMAP4 Panda board.
 * See omap4_common.h for OMAP4 common part
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_PANDA_H
#define __CONFIG_PANDA_H

/*
 * High Level Configuration Options
 */

/* USB UHH support options */
#define CONFIG_CMD_USB
#define CONFIG_USB_HOST
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_OMAP
#define CONFIG_USB_STORAGE
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 3

#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO 1
#define CONFIG_OMAP_EHCI_PHY2_RESET_GPIO 62

/* USB Networking options */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX

#define CONFIG_UBOOT_ENABLE_PADS_ALL

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#define CONFIG_USB_ULPI
#define CONFIG_USB_ULPI_VIEWPORT_OMAP

#include <configs/omap4_common.h>
#define CONFIG_CMD_NET

/* GPIO */
#define CONFIG_CMD_GPIO

/* ENV related config options */
#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_SYS_PROMPT		"Panda # "

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#endif /* __CONFIG_PANDA_H */
