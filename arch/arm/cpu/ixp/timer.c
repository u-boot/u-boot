/*
 * (C) Copyright 2010
 * Michael Schwingen, michael@schwingen.org
 *
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
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

#include <common.h>
#include <asm/arch/ixp425.h>
#include <asm/io.h>
#include <div64.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The IXP42x time-stamp timer runs at 2*OSC_IN (66.666MHz when using a
 * 33.333MHz crystal).
 */
static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, CONFIG_IXP425_TIMER_CLK);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	time *= CONFIG_IXP425_TIMER_CLK;
	do_div(time, CONFIG_SYS_HZ);
	return time;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us = us * CONFIG_IXP425_TIMER_CLK + 999999;
	do_div(us, 1000000);
	return us;
}

unsigned long long get_ticks(void)
{
	ulong now = readl(IXP425_OSTS_B);

	if (readl(IXP425_OSST) & IXP425_OSST_TIMER_TS_PEND) {
		/* rollover of timestamp timer register */
		gd->timestamp += (0xFFFFFFFF - gd->lastinc) + now + 1;
		writel(IXP425_OSST_TIMER_TS_PEND, IXP425_OSST);
	} else {
		/* move stamp forward with absolut diff ticks */
		gd->timestamp += (now - gd->lastinc);
	}
	gd->lastinc = now;
	return gd->timestamp;
}


void reset_timer_masked(void)
{
	/* capture current timestamp counter */
	gd->lastinc = readl(IXP425_OSTS_B);
	/* start "advancing" time stamp from 0 */
	gd->timestamp = 0;
}

ulong get_timer_masked(void)
{
	return tick_to_time(get_ticks());
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* delay x useconds AND preserve advance timestamp value */
void __udelay(unsigned long usec)
{
	unsigned long long tmp;

	tmp = get_ticks() + us_to_tick(usec);

	while (get_ticks() < tmp)
		;
}

int timer_init(void)
{
	writel(IXP425_OSST_TIMER_TS_PEND, IXP425_OSST);
	return 0;
}
