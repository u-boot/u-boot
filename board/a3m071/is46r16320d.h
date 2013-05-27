/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define SDRAM_DDR		/* is DDR */

#if defined(CONFIG_MPC5200)
/* Settings for XLB = 132 MHz */
/* see is46r16320d datasheet and MPC5200UM chap. 8.6.1. */

/* SDRAM Config Standard timing */
#define SDRAM_MODE	0x008d0000
#define SDRAM_EMODE	0x40010000
#define SDRAM_CONTROL	0x70430f00
#define SDRAM_CONFIG1	0x33622930
#define SDRAM_CONFIG2	0x46670000
#define SDRAM_TAPDELAY	0x10000000

#else
#error CONFIG_MPC5200 not defined
#endif
