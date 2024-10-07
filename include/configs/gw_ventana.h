/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Gateworks Corporation
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

/* Serial */
#define CFG_MXC_UART_BASE	       UART2_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR      0

/* PMIC */
#define CFG_POWER_PFUZE100_I2C_ADDR	0x08
#define CFG_POWER_LTC3676_I2C_ADDR  0x3c

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#endif			       /* __CONFIG_H */
