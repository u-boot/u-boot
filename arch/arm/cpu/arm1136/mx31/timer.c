/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
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
#include <asm/arch/mx31-regs.h>
#include <div64.h>

#define TIMER_BASE 0x53f90000 /* General purpose timer 1 */

/* General purpose timers registers */
#define GPTCR	__REG(TIMER_BASE)		/* Control register	*/
#define GPTPR	__REG(TIMER_BASE + 0x4)		/* Prescaler register	*/
#define GPTSR	__REG(TIMER_BASE + 0x8)		/* Status register	*/
#define GPTCNT	__REG(TIMER_BASE + 0x24)	/* Counter register	*/

/* General purpose timers bitfields */
#define GPTCR_SWR		(1 << 15)	/* Software reset	*/
#define GPTCR_FRR		(1 << 9)	/* Freerun / restart	*/
#define GPTCR_CLKSOURCE_32	(4 << 6)	/* Clock source		*/
#define GPTCR_TEN		1		/* Timer enable		*/

static ulong timestamp;
static ulong lastinc;

/* "time" is measured in 1 / CONFIG_SYS_HZ seconds, "tick" is internal timer period */
#ifdef CONFIG_MX31_TIMER_HIGH_PRECISION
/* ~0.4% error - measured with stop-watch on 100s boot-delay */
static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, CONFIG_MX31_CLK32);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	time *= CONFIG_MX31_CLK32;
	do_div(time, CONFIG_SYS_HZ);
	return time;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us = us * CONFIG_MX31_CLK32 + 999999;
	do_div(us, 1000000);
	return us;
}
#else
/* ~2% error */
#define TICK_PER_TIME	((CONFIG_MX31_CLK32 + CONFIG_SYS_HZ / 2) / CONFIG_SYS_HZ)
#define US_PER_TICK	(1000000 / CONFIG_MX31_CLK32)

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

/* The 32768Hz 32-bit timer overruns in 131072 seconds */
int timer_init (void)
{
	int i;

	/* setup GP Timer 1 */
	GPTCR = GPTCR_SWR;
	for (i = 0; i < 100; i++)
		GPTCR = 0; /* We have no udelay by now */
	GPTPR = 0; /* 32Khz */
	/* Freerun Mode, PERCLK1 input */
	GPTCR |= GPTCR_CLKSOURCE_32 | GPTCR_TEN;

	return 0;
}

void reset_timer_masked (void)
{
	/* reset time */
	lastinc = GPTCNT; /* capture current incrementer value time */
	timestamp = 0; /* start "advancing" time stamp from 0 */
}

void reset_timer(void)
{
	reset_timer_masked();
}

unsigned long long get_ticks (void)
{
	ulong now = GPTCNT; /* current tick value */

	if (now >= lastinc)	/* normal mode (non roll) */
		/* move stamp forward with absolut diff ticks */
		timestamp += (now - lastinc);
	else			/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	lastinc = now;
	return timestamp;
}

ulong get_timer_masked (void)
{
	/*
	 * get_ticks() returns a long long (64 bit), it wraps in
	 * 2^64 / CONFIG_MX31_CLK32 = 2^64 / 2^15 = 2^49 ~ 5 * 10^14 (s) ~
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

/* delay x useconds AND preserve advance timestamp value */
void __udelay (unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

void reset_cpu (ulong addr)
{
	__REG16(WDOG_BASE) = 4;
}
