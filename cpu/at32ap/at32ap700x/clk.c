/*
 * Copyright (C) 2005-2008 Atmel Corporation
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

#include <asm/io.h>

#include <asm/arch/clk.h>
#include <asm/arch/memory-map.h>

#include "sm.h"

void clk_init(void)
{
	uint32_t cksel;

	/* in case of soft resets, disable watchdog */
	sm_writel(WDT_CTRL, SM_BF(KEY, 0x55));
	sm_writel(WDT_CTRL, SM_BF(KEY, 0xaa));

#ifdef CONFIG_PLL
	/* Initialize the PLL */
	sm_writel(PM_PLL0, (SM_BF(PLLCOUNT, CFG_PLL0_SUPPRESS_CYCLES)
			    | SM_BF(PLLMUL, CFG_PLL0_MUL - 1)
			    | SM_BF(PLLDIV, CFG_PLL0_DIV - 1)
			    | SM_BF(PLLOPT, CFG_PLL0_OPT)
			    | SM_BF(PLLOSC, 0)
			    | SM_BIT(PLLEN)));

	/* Wait for lock */
	while (!(sm_readl(PM_ISR) & SM_BIT(LOCK0))) ;
#endif

	/* Set up clocks for the CPU and all peripheral buses */
	cksel = 0;
	if (CFG_CLKDIV_CPU)
		cksel |= SM_BIT(CPUDIV) | SM_BF(CPUSEL, CFG_CLKDIV_CPU - 1);
	if (CFG_CLKDIV_HSB)
		cksel |= SM_BIT(HSBDIV) | SM_BF(HSBSEL, CFG_CLKDIV_HSB - 1);
	if (CFG_CLKDIV_PBA)
		cksel |= SM_BIT(PBADIV) | SM_BF(PBASEL, CFG_CLKDIV_PBA - 1);
	if (CFG_CLKDIV_PBB)
		cksel |= SM_BIT(PBBDIV) | SM_BF(PBBSEL, CFG_CLKDIV_PBB - 1);
	sm_writel(PM_CKSEL, cksel);

#ifdef CONFIG_PLL
	/* Use PLL0 as main clock */
	sm_writel(PM_MCCTRL, SM_BIT(PLLSEL));
#endif
}
