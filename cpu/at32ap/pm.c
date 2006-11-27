/*
 * Copyright (C) 2006 Atmel Corporation
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

#ifdef CFG_POWER_MANAGER
#include <asm/errno.h>
#include <asm/io.h>

#include <asm/arch/memory-map.h>
#include <asm/arch/platform.h>

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

struct clock_domain_state {
	const struct device *bridge;
	unsigned long freq;
	u32 mask;
};
static struct clock_domain_state ckd_state[NR_CLOCK_DOMAINS];

int pm_enable_clock(enum clock_domain_id id, unsigned int index)
{
	const struct clock_domain *ckd = &chip_clock[id];
	struct clock_domain_state *state = &ckd_state[id];

	if (ckd->bridge != NO_DEVICE) {
		state->bridge = get_device(ckd->bridge);
		if (!state->bridge)
			return -EBUSY;
	}

	state->mask |= 1 << index;
	if (gd->sm)
		writel(state->mask, gd->sm->regs + ckd->reg);

	return 0;
}

void pm_disable_clock(enum clock_domain_id id, unsigned int index)
{
	const struct clock_domain *ckd = &chip_clock[id];
	struct clock_domain_state *state = &ckd_state[id];

	state->mask &= ~(1 << index);
	if (gd->sm)
		writel(state->mask, gd->sm->regs + ckd->reg);

	if (ckd->bridge)
		put_device(state->bridge);
}

unsigned long pm_get_clock_freq(enum clock_domain_id domain)
{
	return ckd_state[domain].freq;
}

void pm_init(void)
{
	uint32_t cksel = 0;
	unsigned long main_clock;

	/* Make sure we don't disable any device we're already using */
	get_device(DEVICE_HRAMC);
	get_device(DEVICE_HEBI);

	/* Enable the PICO as well */
	ckd_state[CLOCK_CPU].mask |= 1;

	gd->sm = get_device(DEVICE_SM);
	if (!gd->sm)
		panic("Unable to claim system manager device!\n");

	/* Disable any devices that haven't been explicitly claimed */
	sm_writel(gd->sm, PM_PBB_MASK, ckd_state[CLOCK_PBB].mask);
	sm_writel(gd->sm, PM_PBA_MASK, ckd_state[CLOCK_PBA].mask);
	sm_writel(gd->sm, PM_HSB_MASK, ckd_state[CLOCK_HSB].mask);
	sm_writel(gd->sm, PM_CPU_MASK, ckd_state[CLOCK_CPU].mask);

#ifdef CONFIG_PLL
	/* Initialize the PLL */
	main_clock = (CFG_OSC0_HZ / CFG_PLL0_DIV) * CFG_PLL0_MUL;

	sm_writel(gd->sm, PM_PLL0, (SM_BF(PLLCOUNT, CFG_PLL0_SUPPRESS_CYCLES)
				    | SM_BF(PLLMUL, CFG_PLL0_MUL - 1)
				    | SM_BF(PLLDIV, CFG_PLL0_DIV - 1)
				    | SM_BF(PLLOPT, CFG_PLL0_OPT)
				    | SM_BF(PLLOSC, 0)
				    | SM_BIT(PLLEN)));

	/* Wait for lock */
	while (!(sm_readl(gd->sm, PM_ISR) & SM_BIT(LOCK0))) ;
#else
	main_clock = CFG_OSC0_HZ;
#endif

	/* Set up clocks for the CPU and all peripheral buses */
	if (CFG_CLKDIV_CPU) {
		cksel |= SM_BIT(CPUDIV) | SM_BF(CPUSEL, CFG_CLKDIV_CPU - 1);
		ckd_state[CLOCK_CPU].freq = main_clock / (1 << CFG_CLKDIV_CPU);
	} else {
		ckd_state[CLOCK_CPU].freq = main_clock;
	}
	if (CFG_CLKDIV_HSB) {
		cksel |= SM_BIT(HSBDIV) | SM_BF(HSBSEL, CFG_CLKDIV_HSB - 1);
		ckd_state[CLOCK_HSB].freq = main_clock / (1 << CFG_CLKDIV_HSB);
	} else {
		ckd_state[CLOCK_HSB].freq = main_clock;
	}
	if (CFG_CLKDIV_PBA) {
		cksel |= SM_BIT(PBADIV) | SM_BF(PBASEL, CFG_CLKDIV_PBA - 1);
		ckd_state[CLOCK_PBA].freq = main_clock / (1 << CFG_CLKDIV_PBA);
	} else {
		ckd_state[CLOCK_PBA].freq = main_clock;
	}
	if (CFG_CLKDIV_PBB) {
		cksel |= SM_BIT(PBBDIV) | SM_BF(PBBSEL, CFG_CLKDIV_PBB - 1);
		ckd_state[CLOCK_PBB].freq = main_clock / (1 << CFG_CLKDIV_PBB);
	} else {
		ckd_state[CLOCK_PBB].freq = main_clock;
	}
	sm_writel(gd->sm, PM_CKSEL, cksel);

	/* CFG_HZ currently depends on cpu_hz */
	gd->cpu_hz = ckd_state[CLOCK_CPU].freq;

#ifdef CONFIG_PLL
	/* Use PLL0 as main clock */
	sm_writel(gd->sm, PM_MCCTRL, SM_BIT(PLLSEL));
#endif
}

#endif /* CFG_POWER_MANAGER */
