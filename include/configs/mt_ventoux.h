/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 *
 * Configuration settings for the Teejet mt_ventoux board.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tam3517-common.h"

#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10) + \
					6 * 1024 * 1024)

#define MACH_TYPE_AM3517_MT_VENTOUX	3832
#define CONFIG_MACH_TYPE	MACH_TYPE_AM3517_MT_VENTOUX

#define CONFIG_BOOTDELAY	10
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_AUTO_COMPLETE

#define CONFIG_OMAP3_GPIO_4
#define CONFIG_HOSTNAME mt_ventoux

/*
 * Miscellaneous configurable options
 */
#define V_PROMPT			"mt_ventoux => "
#define CONFIG_SYS_PROMPT		V_PROMPT

/*
 * Set its own mtdparts, different from common
 */
#undef MTDIDS_DEFAULT
#undef MTDPARTS_DEFAULT
#define MTDIDS_DEFAULT		"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT	"mtdparts=omap2-nand.0:512k(MLO)," \
				"1m(u-boot),256k(env1)," \
				"256k(env2),8m(ubisystem),-(rootfs)"

/*
 * FPGA
 */
#define CONFIG_CMD_FPGA
#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_SPARTAN3
#define CONFIG_SYS_FPGA_PROG_FEEDBACK
#define CONFIG_SYS_FPGA_WAIT	10000
#define CONFIG_MAX_FPGA_DEVICES	1
#define CONFIG_FPGA_DELAY() udelay(1)
#define CONFIG_SYS_FPGA_PROG_FEEDBACK

#define CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SPLASH_SCREEN
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_CMD_BMP
#define CONFIG_VIDEO_OMAP3	/* DSS Support			*/
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define	CONFIG_EXTRA_ENV_SETTINGS	CONFIG_TAM3517_SETTINGS \
	"bootcmd=run net_nfs\0"

#endif /* __CONFIG_H */
