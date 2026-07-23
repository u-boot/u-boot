/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 *           2018 Emcraft Systems
 *           2022 Purism
 *           2026 Phosh.mobi e.V.
 */

#ifndef __LIBREM5_DEVKIT_H
#define __LIBREM5_DEVKIT_H

/* #define DEBUG */

#include <version.h>
#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_FSL_USDHC_NUM	2

/* UART */
#define CFG_MXC_UART_BASE		UART1_BASE_ADDR

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR		0x40000000
#define CFG_SYS_INIT_RAM_SIZE		0x80000

#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0xc0000000 /* 3GB LPDDR4 one Rank */

/* Monitor Command Prompt */

#define CFG_SYS_FSL_ESDHC_ADDR		0

#endif
