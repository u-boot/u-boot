/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 *
 * Configuration settings for the E+L i.MX6Q DO82 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __EL_ZC5601_H
#define __EL_ZC5601_H


#define CONFIG_MXC_UART_BASE	UART2_BASE
#define CONFIG_CONSOLE_DEV		"ttymxc1"
#define CONFIG_MMCROOT			"/dev/mmcblk0p1"

#define CONFIG_DEFAULT_FDT_FILE	"imx6q-zc5601.dtb"

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */

#include "el6x_common.h"

/* Ethernet */
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE				ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE			RGMII
#define CONFIG_ETHPRIME				"FEC"
#define CONFIG_FEC_MXC_PHYADDR			0x10
#define CONFIG_PHYLIB
#define CONFIG_FEC_FIXED_SPEED			1000 /* No autoneg, fix Gb */

#endif                         /*__EL6Q_CONFIG_H */
