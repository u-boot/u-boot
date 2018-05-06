/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * Configuration for the Technexion twister board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tam3517-common.h"

#define CONFIG_MACH_TYPE	MACH_TYPE_TAM3517

#define CONFIG_TAM3517_SW3_SETTINGS
#define CONFIG_XR16L2751


#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_HOSTNAME "twister"

#define	CONFIG_EXTRA_ENV_SETTINGS	CONFIG_TAM3517_SETTINGS \
	"bootcmd=run nandboot\0"

/* SPL OS boot options */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x00200000

#define CONFIG_SYS_SPL_ARGS_ADDR	(PHYS_SDRAM_1 + 0x100)

#endif /* __CONFIG_H */
