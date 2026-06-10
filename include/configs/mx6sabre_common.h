/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 */

#ifndef __MX6QSABRE_COMMON_CONFIG_H
#define __MX6QSABRE_COMMON_CONFIG_H

#include "mx6_common.h"

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR      0

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/* Environment organization */

#endif                         /* __MX6QSABRE_COMMON_CONFIG_H */
