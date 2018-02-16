/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 *
 * Configuration settings for the Teejet mt_ventoux board.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tam3517-common.h"

#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10) + \
					6 * 1024 * 1024)

#define CONFIG_MACH_TYPE	MACH_TYPE_AM3517_MT_VENTOUX

#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_HOSTNAME mt_ventoux

/*
 * Set its own mtdparts, different from common
 */

/*
 * FPGA
 */
#define CONFIG_SYS_FPGA_PROG_FEEDBACK
#define CONFIG_SYS_FPGA_WAIT	10000
#define CONFIG_MAX_FPGA_DEVICES	1
#define CONFIG_FPGA_DELAY() udelay(1)
#define CONFIG_SYS_FPGA_PROG_FEEDBACK

#define CONFIG_SPLASH_SCREEN
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_OMAP3	/* DSS Support			*/

#define	CONFIG_EXTRA_ENV_SETTINGS	CONFIG_TAM3517_SETTINGS \
	"bootcmd=run net_nfs\0"

#endif /* __CONFIG_H */
