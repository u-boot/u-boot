/*
 * Copyright (C) 2005-2006 Atmel Corporation
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
#include <command.h>

#include <asm/io.h>
#include <asm/sections.h>
#include <asm/sysreg.h>

#include <asm/arch/clk.h>
#include <asm/arch/memory-map.h>

#include "hsmc3.h"
#include "sm.h"

/* Sanity checks */
#if (CFG_CLKDIV_CPU > CFG_CLKDIV_HSB)		\
	|| (CFG_CLKDIV_HSB > CFG_CLKDIV_PBA)	\
	|| (CFG_CLKDIV_HSB > CFG_CLKDIV_PBB)
# error Constraint fCPU >= fHSB >= fPB{A,B} violated
#endif
#if defined(CONFIG_PLL) && ((CFG_PLL0_MUL < 1) || (CFG_PLL0_DIV < 1))
# error Invalid PLL multiplier and/or divider
#endif

DECLARE_GLOBAL_DATA_PTR;

static void pm_init(void)
{
	uint32_t cksel;

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

	gd->cpu_hz = get_cpu_clk_rate();

#ifdef CONFIG_PLL
	/* Use PLL0 as main clock */
	sm_writel(PM_MCCTRL, SM_BIT(PLLSEL));
#endif
}

int cpu_init(void)
{
	extern void _evba(void);
	char *p;

	gd->cpu_hz = CFG_OSC0_HZ;

	/* TODO: Move somewhere else, but needs to be run before we
	 * increase the clock frequency. */
	hsmc3_writel(MODE0, 0x00031103);
	hsmc3_writel(CYCLE0, 0x000c000d);
	hsmc3_writel(PULSE0, 0x0b0a0906);
	hsmc3_writel(SETUP0, 0x00010002);

	pm_init();

	sysreg_write(EVBA, (unsigned long)&_evba);
	asm volatile("csrf	%0" : : "i"(SYSREG_EM_OFFSET));

	/* Lock everything that mess with the flash in the icache */
	for (p = __flashprog_start; p <= (__flashprog_end + CFG_ICACHE_LINESZ);
	     p += CFG_ICACHE_LINESZ)
		asm volatile("cache %0, 0x02" : "=m"(*p) :: "memory");

	return 0;
}

void prepare_to_boot(void)
{
	/* Flush both caches and the write buffer */
	asm volatile("cache  %0[4], 010\n\t"
		     "cache  %0[0], 000\n\t"
		     "sync   0" : : "r"(0) : "memory");
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	/* This will reset the CPU core, caches, MMU and all internal busses */
	__builtin_mtdr(8, 1 << 13);	/* set DC:DBE */
	__builtin_mtdr(8, 1 << 30);	/* set DC:RES */

	/* Flush the pipeline before we declare it a failure */
	asm volatile("sub   pc, pc, -4");

	return -1;
}
