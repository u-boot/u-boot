/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7ULP EVK board.
 */

#ifndef __MX7ULP_EVK_CONFIG_H
#define __MX7ULP_EVK_CONFIG_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_HZ_CLOCK		1000000 /* Fixed at 1Mhz from TSTMR */

/* UART */
#define LPUART_BASE			LPUART4_RBASE

/* Miscellaneous configurable options */

/* Physical Memory Map */

#define PHYS_SDRAM			0x60000000
#define PHYS_SDRAM_SIZE			SZ_1G
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	SZ_256K

#endif	/* __CONFIG_H */
