/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated.
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 *
 * Configuration settings for the TI SDP4430 board.
 * See ti_omap4_common.h for OMAP4 common part
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_SDP4430_H
#define __CONFIG_SDP4430_H

/*
 * High Level Configuration Options
 */
#define CONFIG_4430SDP		1	/* working with SDP */
#define CONFIG_MACH_TYPE	MACH_TYPE_OMAP_4430SDP

#include <configs/ti_omap4_common.h>

/* Battery Charger */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_CMD_BAT			1
#endif

/* ENV related config options */
#define CONFIG_ENV_IS_IN_MMC		1
#define CONFIG_SYS_MMC_ENV_DEV		1	/* SLOT2: eMMC(1) */
#define CONFIG_ENV_OFFSET		0xE0000
#define CONFIG_CMD_SAVEENV

#endif /* __CONFIG_SDP4430_H */
