/* SPDX-License-Identifier: GPL-2.0+ */
// Copyright (C) Stefano Babic <sbabic@denx.de>

#ifndef __LXR2_CONFIG_H
#define __LXR2_CONFIG_H

#include <config_distro_bootcmd.h>

#include "mx6_common.h"

#define PHYS_SDRAM_SIZE			SZ_1G

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR		IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE		IRAM_SIZE

#define CFG_SYS_FSL_ESDHC_ADDR		0
#define CFG_MXC_UART_BASE		UART4_BASE

#endif
