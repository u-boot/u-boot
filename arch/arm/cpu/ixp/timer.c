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
 * SPDX-License-Identifier:	GPL-2.0+
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
		gd->arch.timestamp += (0xFFFFFFFF - gd->arch.lastinc) + now + 1;
		writel(IXP425_OSST_TIMER_TS_PEND, IXP425_OSST);
	} else {
		/* move stamp forward with absolut diff ticks */
		gd->arch.timestamp += (now - gd->arch.lastinc);
	}
	gd->arch.lastinc = now;
	return gd->arch.timestamp;
}


void reset_timer_masked(void)
{
	/* capture current timestamp counter */
	gd->arch.lastinc = readl(IXP425_OSTS_B);
	/* start "advancing" time stamp from 0 */
	gd->arch.timestamp = 0;
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
