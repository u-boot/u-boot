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
#include <asm/arch/tegra2.h>

DECLARE_GLOBAL_DATA_PTR;

struct timerus *timer_base = (struct timerus *)NV_PA_TMRUS_BASE;

/* counter runs at 1MHz */
#define TIMER_CLK	(1000000)
#define TIMER_LOAD_VAL	0xffffffff

/* timer without interrupts */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	long tmo = usec * (TIMER_CLK / 1000) / 1000;
	unsigned long now, last = readl(&timer_base->cntr_1us);

	while (tmo > 0) {
		now = readl(&timer_base->cntr_1us);
		if (last > now) /* count up timer overflow */
			tmo -= TIMER_LOAD_VAL - last + now;
		else
			tmo -= now - last;
		last = now;
	}
}

void reset_timer_masked(void)
{
	/* reset time, capture current incrementer value time */
	gd->lastinc = readl(&timer_base->cntr_1us) / (TIMER_CLK/CONFIG_SYS_HZ);
	gd->tbl = 0;		/* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	ulong now;

	/* current tick value */
	now = readl(&timer_base->cntr_1us) / (TIMER_CLK / CONFIG_SYS_HZ);

	if (now >= gd->lastinc)	/* normal mode (non roll) */
		/* move stamp forward with absolute diff ticks */
		gd->tbl += (now - gd->lastinc);
	else	/* we have rollover of incrementer */
		gd->tbl += ((TIMER_LOAD_VAL / (TIMER_CLK / CONFIG_SYS_HZ))
				- gd->lastinc) + now;
	gd->lastinc = now;
	return gd->tbl;
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
