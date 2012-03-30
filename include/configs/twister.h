/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * Configuration for the Technexion twister board.
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
 * Foundation, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tam3517-common.h"

#define MACH_TYPE_TAM3517	2818
#define CONFIG_MACH_TYPE	MACH_TYPE_TAM3517

#define CONFIG_TAM3517_SW3_SETTINGS
#define CONFIG_XR16L2751

#define CONFIG_BOOTDELAY	10

#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_HOSTNAME twister

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_PROMPT		"twister => "

#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#define CONFIG_SMC911X_BASE		0x2C000000
#define CONFIG_SMC911X_NO_EEPROM

#define	CONFIG_EXTRA_ENV_SETTINGS	CONFIG_TAM3517_SETTINGS \
	"bootcmd=run nandboot\0"

/* SPL OS boot options */
#define CONFIG_CMD_SPL
#define CONFIG_CMD_SPL_WRITE_SIZE	0x400 /* 1024 byte */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x00200000
#define CONFIG_CMD_SPL_NAND_OFS	(CONFIG_SYS_NAND_SPL_KERNEL_OFFS+\
						0x600000)
#define CONFIG_SPL_OS_BOOT
#define CONFIG_SPL_OS_BOOT_KEY	55

#define CONFIG_SYS_SPL_ARGS_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_SPL_BOARD_INIT

#endif /* __CONFIG_H */
