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

/* High Level Configuration Options */
#define CONFIG_DRA7XX		/* in a TI DRA7XX core */
#define CONFIG_ENV_IS_NOWHERE		/* For now. */

#include <configs/omap5_common.h>

#define CONFIG_SYS_PROMPT		"DRA752 EVM # "

#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		UART1_BASE
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_OMAP_ABE_SYSCK

#define CONSOLEDEV		"ttyO0"

#endif /* __CONFIG_DRA7XX_EVM_H */
