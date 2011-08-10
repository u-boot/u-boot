/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <faraday/ftpmu010.h>
#include <faraday/fttmr010.h>

DECLARE_GLOBAL_DATA_PTR;

#define TIMER_CLOCK	32768
#define TIMER_LOAD_VAL	0xffffffff

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, gd->timer_rate_hz);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= gd->timer_rate_hz;
	do_div(usec, 1000000);

	return usec;
}

int timer_init(void)
{
	struct fttmr010 *tmr = (struct fttmr010 *)CONFIG_FTTMR010_BASE;
	unsigned int cr;

	debug("%s()\n", __func__);

	/* disable timers */
	writel(0, &tmr->cr);

	/* use 32768Hz oscillator for RTC, WDT, TIMER */
	ftpmu010_32768osc_enable();

	/* setup timer */
	writel(TIMER_LOAD_VAL, &tmr->timer3_load);
	writel(TIMER_LOAD_VAL, &tmr->timer3_counter);
	writel(0, &tmr->timer3_match1);
	writel(0, &tmr->timer3_match2);

	/* we don't want timer to issue interrupts */
	writel(FTTMR010_TM3_MATCH1 |
	       FTTMR010_TM3_MATCH2 |
	       FTTMR010_TM3_OVERFLOW,
	       &tmr->interrupt_mask);

	cr = readl(&tmr->cr);
	cr |= FTTMR010_TM3_CLOCK;	/* use external clock */
	cr |= FTTMR010_TM3_ENABLE;
	writel(cr, &tmr->cr);

	gd->timer_rate_hz = TIMER_CLOCK;
	gd->tbu = gd->tbl = 0;

	return 0;
}

/*
 * Get the current 64 bit timer tick count
 */
unsigned long long get_ticks(void)
{
	struct fttmr010 *tmr = (struct fttmr010 *)CONFIG_FTTMR010_BASE;
	ulong now = TIMER_LOAD_VAL - readl(&tmr->timer3_counter);

	/* increment tbu if tbl has rolled over */
	if (now < gd->tbl)
		gd->tbu++;
	gd->tbl = now;
	return (((unsigned long long)gd->tbu) << 32) | gd->tbl;
}

void __udelay(unsigned long usec)
{
	unsigned long long start;
	ulong tmo;

	start = get_ticks();		/* get current timestamp */
	tmo = usec_to_tick(usec);	/* convert usecs to ticks */
	while ((get_ticks() - start) < tmo)
		;			/* loop till time has passed */
}

/*
 * get_timer(base) can be used to check for timeouts or
 * to measure elasped time relative to an event:
 *
 * ulong start_time = get_timer(0) sets start_time to the current
 * time value.
 * get_timer(start_time) returns the time elapsed since then.
 *
 * The time is used in CONFIG_SYS_HZ units!
 */
ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

/*
 * Return the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->timer_rate_hz;
}
