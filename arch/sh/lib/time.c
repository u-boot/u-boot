/*
 * (C) Copyright 2009
 * Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * (C) Copyright 2007-2010
 * Nobobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <asm/processor.h>
#include <asm/clk.h>
#include <asm/io.h>

#define TMU_MAX_COUNTER (~0UL)

static ulong timer_freq;
static unsigned long last_tcnt;
static unsigned long long overflow_ticks;

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, timer_freq);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= timer_freq;
	do_div(usec, 1000000);

	return usec;
}

static void tmu_timer_start (unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(TSTR) | (1 << timer), TSTR);
}

static void tmu_timer_stop (unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(TSTR) & ~(1 << timer), TSTR);
}

int timer_init (void)
{
	/* Divide clock by CONFIG_SYS_TMU_CLK_DIV */
	u16 bit = 0;

	switch (CONFIG_SYS_TMU_CLK_DIV) {
	case 1024:
		bit = 4;
		break;
	case 256:
		bit = 3;
		break;
	case 64:
		bit = 2;
		break;
	case 16:
		bit = 1;
		break;
	case 4:
	default:
		break;
	}
	writew(readw(TCR0) | bit, TCR0);

	/* Calc clock rate */
	timer_freq = get_tmu0_clk_rate() >> ((bit + 1) * 2);

	tmu_timer_stop(0);
	tmu_timer_start(0);

	last_tcnt = 0;
	overflow_ticks = 0;

	return 0;
}

unsigned long long get_ticks (void)
{
	unsigned long tcnt = 0 - readl(TCNT0);
	unsigned long ticks;

	if (last_tcnt > tcnt) { /* overflow */
		overflow_ticks++;
		ticks = (0xffffffff - last_tcnt) + tcnt;
	} else {
		ticks = tcnt;
	}
	last_tcnt = tcnt;

	return (overflow_ticks << 32) | tcnt;
}

void __udelay (unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = usec_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

unsigned long get_timer (unsigned long base)
{
	/* return msec */
	return tick_to_time(get_ticks()) - base;
}

unsigned long get_tbclk (void)
{
	return timer_freq;
}
