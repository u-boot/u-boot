/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreAuto board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MX6QSABREAUTO_CONFIG_H
#define __MX6QSABREAUTO_CONFIG_H

#define CONFIG_MACH_TYPE	3529
#define CONFIG_MXC_UART_BASE	UART4_BASE
#define CONFIG_CONSOLE_DEV		"ttymxc3"
#define CONFIG_DEFAULT_FDT_FILE	"imx6q-sabreauto.dtb"
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"
#define PHYS_SDRAM_SIZE		(2u * 1024 * 1024 * 1024)

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#include "mx6sabre_common.h"

#define CONFIG_SYS_FSL_USDHC_NUM	2
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		100000

#endif                         /* __MX6QSABREAUTO_CONFIG_H */
