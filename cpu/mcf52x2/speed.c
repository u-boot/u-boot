/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * get_clocks() fills in gd->cpu_clock and gd->bus_clk
 */
int get_clocks (void)
{
#if defined(CONFIG_M5249) || defined(CONFIG_M5253)
	volatile unsigned long cpll = mbar2_readLong(MCFSIM_PLLCR);
	unsigned long pllcr;

#ifndef CFG_PLL_BYPASS

#ifdef CONFIG_M5249
	/* Setup the PLL to run at the specified speed */
#ifdef CFG_FAST_CLK
	pllcr = 0x925a3100;	/* ~140MHz clock (PLL bypass = 0) */
#else
	pllcr = 0x135a4140;	/* ~72MHz clock (PLL bypass = 0) */
#endif
#endif				/* CONFIG_M5249 */

#ifdef CONFIG_M5253
	pllcr = CFG_PLLCR;
#endif				/* CONFIG_M5253 */

	cpll = cpll & 0xfffffffe;	/* Set PLL bypass mode = 0 (PSTCLK = crystal) */
	mbar2_writeLong(MCFSIM_PLLCR, cpll);	/* Set the PLL to bypass mode (PSTCLK = crystal) */
	mbar2_writeLong(MCFSIM_PLLCR, pllcr);	/* set the clock speed */
	pllcr ^= 0x00000001;	/* Set pll bypass to 1 */
	mbar2_writeLong(MCFSIM_PLLCR, pllcr);	/* Start locking (pll bypass = 1) */
	udelay(0x20);		/* Wait for a lock ... */
#endif				/* #ifndef CFG_PLL_BYPASS */

#endif				/* CONFIG_M5249 || CONFIG_M5253 */

#if defined(CONFIG_M5275)
	volatile pll_t *pll = (volatile pll_t *)(MMAP_PLL);

	/* Setup PLL */
	pll->syncr = 0x01080000;
	while (!(pll->synsr & FMPLL_SYNSR_LOCK)
		;
	pll->syncr = 0x01000000;
	while (!(pll->synsr & FMPLL_SYNSR_LOCK))
		;
#endif

	gd->cpu_clk = CFG_CLK;
#if defined(CONFIG_M5249) || defined(CONFIG_M5253) || defined(CONFIG_M5275)
	gd->bus_clk = gd->cpu_clk / 2;
#else
	gd->bus_clk = gd->cpu_clk;
#endif
	return (0);
}
