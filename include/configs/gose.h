/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/gose.h
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 */

#ifndef __GOSE_H
#define __GOSE_H

#include "rcar-gen2-common.h"

#define CONFIG_SYS_INIT_SP_ADDR		0x4f000000
#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(1048u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(512u * 1024 * 1024)

/* SH Ether */
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0x1
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_RMII
#define CONFIG_SH_ETHER_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_CACHE_INVALIDATE
#define CONFIG_SH_ETHER_ALIGNE_SIZE	64

/* Board Clock */

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"bootm_size=0x10000000\0"

/* SPL support */
#define CONFIG_SPL_STACK		0xe6340000
#define CONFIG_SPL_MAX_SIZE		0x4000

#endif	/* __GOSE_H */
