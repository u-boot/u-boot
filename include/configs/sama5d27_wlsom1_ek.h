/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA5D27 WLSOM1 EK Board.
 *
 * Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Nicolas Ferre <nicolas.ferre@microcihp.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#undef CONFIG_SYS_AT91_MAIN_CLOCK
#define CONFIG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* NAND flash */
#undef CONFIG_CMD_NAND

#ifdef CONFIG_SD_BOOT
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#endif

#endif
