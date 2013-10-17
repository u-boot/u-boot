/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define SDRAM_DDR		/* is DDR */

#if defined(CONFIG_MPC5200)
/* Settings for XLB = 132 MHz */
#define SDRAM_MODE	0x018D0000
#define SDRAM_EMODE	0x40090000
#define SDRAM_CONTROL	0x704f0f00
#define SDRAM_CONFIG1	0x73722930
#define SDRAM_CONFIG2	0x47770000
#define SDRAM_TAPDELAY	0x10000000

#else
#error CONFIG_MPC5200 not defined
#endif
