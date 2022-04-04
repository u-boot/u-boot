/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Compact Integrator board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "integrator-common.h"

/* Integrator CP-specific configuration */
#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer 1 is clocked at 1Mhz */

/*
 * Hardware drivers
 */
#define CONFIG_SMC91111
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC91111_BASE    0xC8000000
#undef CONFIG_SMC91111_EXT_PHY

#define CONFIG_SERVERIP 192.168.1.100
#define CONFIG_IPADDR 192.168.1.104

/*
 * Miscellaneous configurable options
 */
#define PHYS_FLASH_SIZE			0x01000000	/* 16MB */
#define CONFIG_SYS_MAX_FLASH_SECT	64
#define CONFIG_SYS_MONITOR_LEN		0x00100000

#endif /* __CONFIG_H */
