/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

#include <asm/arch/pxa-regs.h>
#include <asm/io.h>
#include <common.h>
#include <div64.h>

#ifdef CONFIG_USE_IRQ
#error: interrupts not implemented yet
#endif

#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define TIMER_FREQ_HZ 3250000
#elif defined(CONFIG_PXA250)
#define TIMER_FREQ_HZ 3686400
#else
#error "Timer frequency unknown - please config PXA CPU type"
#endif

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, TIMER_FREQ_HZ);
	return tick;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us = us * TIMER_FREQ_HZ + 999999;
	do_div(us, 1000000);
	return us;
}

int timer_init (void)
{
	reset_timer();

	return 0;
}

void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void __udelay (unsigned long usec)
{
	udelay_masked (usec);
}


void reset_timer_masked (void)
{
	writel(0, OSCR);
}

ulong get_timer_masked (void)
{
	return tick_to_time(get_ticks());
}

void udelay_masked (unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;

}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return readl(OSCR);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;
	tbclk = TIMER_FREQ_HZ;
	return tbclk;
}
