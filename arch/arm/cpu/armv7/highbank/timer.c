/*
 * Copyright 2010-2011 Calxeda, Inc.
 *
 * Based on arm926ejs/mx27/timer.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <div64.h>
#include <linux/types.h>        /* for size_t */
#include <linux/stddef.h>       /* for NULL */
#include <asm/io.h>
#include <asm/arch-armv7/systimer.h>

#undef SYSTIMER_BASE
#define SYSTIMER_BASE		0xFFF34000	/* Timer 0 and 1 base	*/
#define SYSTIMER_RATE		150000000

static ulong timestamp;
static ulong lastinc;
static struct systimer *systimer_base = (struct systimer *)SYSTIMER_BASE;

/*
 * Start the timer
 */
int timer_init(void)
{
	/*
	 * Setup timer0
	 */
	writel(SYSTIMER_RELOAD, &systimer_base->timer0load);
	writel(SYSTIMER_RELOAD, &systimer_base->timer0value);
	writel(SYSTIMER_EN | SYSTIMER_32BIT, &systimer_base->timer0control);

	reset_timer_masked();

	return 0;

}

#define TICK_PER_TIME	((SYSTIMER_RATE + CONFIG_SYS_HZ / 2) / CONFIG_SYS_HZ)
#define NS_PER_TICK	(1000000000 / SYSTIMER_RATE)

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	do_div(tick, TICK_PER_TIME);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	return time * TICK_PER_TIME;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	unsigned long long tick = us * 1000;
	tick += NS_PER_TICK - 1;
	do_div(tick, NS_PER_TICK);
	return tick;
}

unsigned long long get_ticks(void)
{
	ulong now = ~readl(&systimer_base->timer0value);

	if (now >= lastinc)	/* normal mode (non roll) */
		/* move stamp forward with absolut diff ticks */
		timestamp += (now - lastinc);
	else			/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	lastinc = now;
	return timestamp;
}

/*
 * Delay x useconds AND preserve advance timstamp value
 *     assumes timer is ticking at 1 msec
 */
void __udelay(ulong usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked(void)
{
	lastinc = ~readl(&systimer_base->timer0value);
	timestamp = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer_masked(void)
{
	return tick_to_time(get_ticks());
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
