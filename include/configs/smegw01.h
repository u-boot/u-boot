/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 NXP Semiconductors
 * Copyright (C) 2021 Fabio Estevam <festevam@denx.de>
 *
 * Configuration settings for the smegw01 board.
 */

#ifndef __SMEGW01_CONFIG_H
#define __SMEGW01_CONFIG_H

#include "mx7_common.h"
#include <imximage.h>

#define PHYS_SDRAM_SIZE		SZ_512M

/* MMC Config*/
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* default to no extra bootparams, we need an empty define for stringification*/
#ifndef EXTRA_BOOTPARAMS
#define EXTRA_BOOTPARAMS
#endif

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif
