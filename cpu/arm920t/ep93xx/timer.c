/*
 * Cirrus Logic EP93xx timer support.
 *
 * Copyright (C) 2009, 2010
 * Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2004, 2005
 * Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
 *
 * Based on the original intr.c Cirrus Logic EP93xx Rev D. interrupt support,
 * author unknown.
 *
 * See file CREDITS for list of people who contributed to this project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <linux/types.h>
#include <asm/arch/ep93xx.h>
#include <asm/io.h>

#define TIMER_CLKSEL	(1 << 3)
#define TIMER_MODE	(1 << 6)
#define TIMER_ENABLE	(1 << 7)

#define TIMER_FREQ	508469
#define TIMER_LOAD_VAL	(TIMER_FREQ / CONFIG_SYS_HZ)

static ulong timestamp;
static ulong lastdec;

static inline unsigned long clk_to_systicks(unsigned long clk_ticks)
{
	unsigned long sys_ticks = (clk_ticks * CONFIG_SYS_HZ) / TIMER_FREQ;

	return sys_ticks;
}

static inline unsigned long usecs_to_ticks(unsigned long usecs)
{
	unsigned long ticks;

	if (usecs >= 1000) {
		ticks = usecs / 1000;
		ticks *= (TIMER_LOAD_VAL * CONFIG_SYS_HZ);
		ticks /= 1000;
	} else {
		ticks = usecs * TIMER_LOAD_VAL * CONFIG_SYS_HZ;
		ticks /= (1000 * 1000);
	}

	return ticks;
}

static inline unsigned long read_timer(void)
{
	struct timer_regs *timer = (struct timer_regs *)TIMER_BASE;

	return readl(&timer->timer3.value);
}

/*
 * timer without interrupts
 */
unsigned long long get_ticks(void)
{
	const unsigned long now = read_timer();

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}

	lastdec = now;

	return timestamp;
}

unsigned long get_timer_masked(void)
{
	return clk_to_systicks(get_ticks());
}

unsigned long get_timer(unsigned long base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked(void)
{
	lastdec = read_timer();
	timestamp = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

void set_timer(unsigned long t)
{
	timestamp = t;
}

void __udelay(unsigned long usec)
{
	const unsigned long ticks = usecs_to_ticks(usec);
	const unsigned long target = clk_to_systicks(ticks) + get_timer(0);

	while (get_timer_masked() < target)
		/* noop */;
}

void udelay_masked(unsigned long usec)
{
	const unsigned long ticks = usecs_to_ticks(usec);
	const unsigned long target = clk_to_systicks(ticks) + get_timer(0);

	reset_timer_masked();

	while (get_timer_masked() < target)
		/* noop */;
}

int timer_init(void)
{
	struct timer_regs *timer = (struct timer_regs *)TIMER_BASE;

	/* use timer 3 with 508KHz and free running */
	writel(TIMER_CLKSEL, &timer->timer3.control);

	/* auto load, manual update of Timer 3 */
	lastdec = TIMER_LOAD_VAL;
	writel(TIMER_LOAD_VAL, &timer->timer3.load);

	/* Enable the timer and periodic mode */
	writel(TIMER_ENABLE | TIMER_MODE | TIMER_CLKSEL,
		&timer->timer3.control);

	reset_timer_masked();

	return 0;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
unsigned long get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
