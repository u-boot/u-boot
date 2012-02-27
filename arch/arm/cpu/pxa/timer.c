/*
 * Marvell PXA2xx/3xx timer driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
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

DECLARE_GLOBAL_DATA_PTR;

#define	TIMER_LOAD_VAL	0xffffffff

#define	timestamp	(gd->tbl)
#define	lastinc		(gd->lastinc)

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define	TIMER_FREQ_HZ	3250000
#elif defined(CONFIG_CPU_PXA25X)
#define	TIMER_FREQ_HZ	3686400
#else
#error "Timer frequency unknown - please config PXA CPU type"
#endif

static unsigned long long tick_to_time(unsigned long long tick)
{
	return tick * CONFIG_SYS_HZ / TIMER_FREQ_HZ;
}

static unsigned long long us_to_tick(unsigned long long us)
{
	return (us * TIMER_FREQ_HZ) / 1000000;
}

int timer_init(void)
{
	writel(0, OSCR);
	return 0;
}

unsigned long long get_ticks(void)
{
	/* Current tick value */
	uint32_t now = readl(OSCR);

	if (now >= lastinc) {
		/*
		 * Normal mode (non roll)
		 * Move stamp forward with absolute diff ticks
		 */
		timestamp += (now - lastinc);
	} else {
		/* We have rollover of incrementer */
		timestamp += (TIMER_LOAD_VAL - lastinc) + now;
	}

	lastinc = now;
	return timestamp;
}

ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

ulong get_tbclk(void)
{
	return TIMER_FREQ_HZ;
}
