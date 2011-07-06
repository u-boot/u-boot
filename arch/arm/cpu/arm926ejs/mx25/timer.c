/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2009
 * Ilya Yanok, Emcraft Systems Ltd, <yanok@emcraft.com>
 *
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 * 	Add support for MX25
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
#include <div64.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

DECLARE_GLOBAL_DATA_PTR;

#define timestamp gd->tbl
#define lastinc gd->lastinc

/*
 * "time" is measured in 1 / CONFIG_SYS_HZ seconds,
 * "tick" is internal timer period
 */
#ifdef CONFIG_MX25_TIMER_HIGH_PRECISION
/* ~0.4% error - measured with stop-watch on 100s boot-delay */
static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, CONFIG_MX25_CLK32);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	time *= CONFIG_MX25_CLK32;
	do_div(time, CONFIG_SYS_HZ);
	return time;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us = us * CONFIG_MX25_CLK32 + 999999;
	do_div(us, 1000000);
	return us;
}
#else
/* ~2% error */
#define TICK_PER_TIME	((CONFIG_MX25_CLK32 + CONFIG_SYS_HZ / 2) / \
		CONFIG_SYS_HZ)
#define US_PER_TICK	(1000000 / CONFIG_MX25_CLK32)

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
	us += US_PER_TICK - 1;
	do_div(us, US_PER_TICK);
	return us;
}
#endif

/* nothing really to do with interrupts, just starts up a counter. */
/* The 32KHz 32-bit timer overruns in 134217 seconds */
int timer_init(void)
{
	int i;
	struct gpt_regs *gpt = (struct gpt_regs *)IMX_GPT1_BASE;
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;

	/* setup GP Timer 1 */
	writel(GPT_CTRL_SWR, &gpt->ctrl);

	writel(readl(&ccm->cgr1) | CCM_CGR1_GPT1, &ccm->cgr1);

	for (i = 0; i < 100; i++)
		writel(0, &gpt->ctrl); /* We have no udelay by now */
	writel(0, &gpt->pre); /* prescaler = 1 */
	/* Freerun Mode, 32KHz input */
	writel(readl(&gpt->ctrl) | GPT_CTRL_CLKSOURCE_32 | GPT_CTRL_FRR,
			&gpt->ctrl);
	writel(readl(&gpt->ctrl) | GPT_CTRL_TEN, &gpt->ctrl);

	return 0;
}

void reset_timer_masked(void)
{
	struct gpt_regs *gpt = (struct gpt_regs *)IMX_GPT1_BASE;
	/* reset time */
	/* capture current incrementer value time */
	lastinc = readl(&gpt->counter);
	timestamp = 0; /* start "advancing" time stamp from 0 */
}

void reset_timer(void)
{
	reset_timer_masked();
}

unsigned long long get_ticks (void)
{
	struct gpt_regs *gpt = (struct gpt_regs *)IMX_GPT1_BASE;
	ulong now = readl(&gpt->counter); /* current tick value */

	if (now >= lastinc) {
		/*
		 * normal mode (non roll)
		 * move stamp forward with absolut diff ticks
		 */
		timestamp += (now - lastinc);
	} else {
		/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	}
	lastinc = now;
	return timestamp;
}

ulong get_timer_masked (void)
{
	/*
	 * get_ticks() returns a long long (64 bit), it wraps in
	 * 2^64 / CONFIG_MX25_CLK32 = 2^64 / 2^15 = 2^49 ~ 5 * 10^14 (s) ~
	 * 5 * 10^9 days... and get_ticks() * CONFIG_SYS_HZ wraps in
	 * 5 * 10^6 days - long enough.
	 */
	return tick_to_time(get_ticks());
}

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void set_timer (ulong t)
{
	timestamp = time_to_tick(t);
}

/* delay x useconds AND preserve advance timstamp value */
void __udelay (unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CONFIG_MX25_CLK32;
	return tbclk;
}
