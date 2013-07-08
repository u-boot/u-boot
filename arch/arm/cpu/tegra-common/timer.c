/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 *
 * (C) Copyright 2008
 * Texas Instruments
 *
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Moahmmed Khasim <khasim@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/timer.h>

DECLARE_GLOBAL_DATA_PTR;

/* counter runs at 1MHz */
#define TIMER_CLK	1000000
#define TIMER_LOAD_VAL	0xffffffff

/* timer without interrupts */
ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	long tmo = usec * (TIMER_CLK / 1000) / 1000;
	unsigned long now, last = timer_get_us();

	while (tmo > 0) {
		now = timer_get_us();
		if (last > now) /* count up timer overflow */
			tmo -= TIMER_LOAD_VAL - last + now;
		else
			tmo -= now - last;
		last = now;
	}
}

ulong get_timer_masked(void)
{
	ulong now;

	/* current tick value */
	now = timer_get_us() / (TIMER_CLK / CONFIG_SYS_HZ);

	if (now >= gd->arch.lastinc)	/* normal mode (non roll) */
		/* move stamp forward with absolute diff ticks */
		gd->arch.tbl += (now - gd->arch.lastinc);
	else	/* we have rollover of incrementer */
		gd->arch.tbl += ((TIMER_LOAD_VAL / (TIMER_CLK / CONFIG_SYS_HZ))
				- gd->arch.lastinc) + now;
	gd->arch.lastinc = now;
	return gd->arch.tbl;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

unsigned long timer_get_us(void)
{
	struct timerus *timer_base = (struct timerus *)NV_PA_TMRUS_BASE;

	return readl(&timer_base->cntr_1us);
}
