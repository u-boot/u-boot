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

/* MMC ENV related defines */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1	/* SLOT2: eMMC(1) */
#define CONFIG_ENV_OFFSET		0xE0000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_CMD_SAVEENV

#define CONSOLEDEV			"ttyO0"
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		UART1_BASE
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_OMAP_ABE_SYSCK

#include <configs/omap5_common.h>

/* CPSW Ethernet */
#define CONFIG_CMD_NET			/* 'bootp' and 'tftp' */
#define CONFIG_CMD_DHCP
#define CONFIG_BOOTP_DNS		/* Configurable parts of CMD_DHCP */
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT		10
#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_DRIVER_TI_CPSW		/* Driver for IP block */
#define CONFIG_MII			/* Required in net/eth.c */
#define CONFIG_PHY_GIGE			/* per-board part of CPSW */
#define CONFIG_PHYLIB
#define CONFIG_PHY_ADDR			2

#endif /* __CONFIG_DRA7XX_EVM_H */
