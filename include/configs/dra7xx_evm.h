/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Lokesh Vutla	  <lokeshvutla@ti.com>
 *
 * Configuration settings for the TI DRA7XX board.
 * See omap5_common.h for omap5 common settings.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __CONFIG_DRA7XX_EVM_H
#define __CONFIG_DRA7XX_EVM_H

#define CONFIG_DRA7XX

#define CONFIG_ENV_IS_NOWHERE		/* For now. */

#define CONSOLEDEV			"ttyO0"
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		UART1_BASE
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_OMAP_ABE_SYSCK

#include <configs/omap5_common.h>

/* CPSW Ethernet */
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_DRIVER_TI_CPSW
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT         10
#define CONFIG_NET_MULTI
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_ADDR			2

#endif /* __CONFIG_DRA7XX_EVM_H */
