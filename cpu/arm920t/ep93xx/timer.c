/*
 * Cirrus Logic EP93xx timer support.
 *
 * Copyright (C) 2009, 2010 Matthias Kaehlcke <matthias@kaehlcke.net>
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
#include <div64.h>

#define TIMER_CLKSEL	(1 << 3)
#define TIMER_ENABLE	(1 << 7)

#define TIMER_FREQ			508469		/* ticks / second */
#define TIMER_MAX_VAL			0xFFFFFFFF

static struct ep93xx_timer
{
	unsigned long long ticks;
	unsigned long last_read;
} timer;

static inline unsigned long long usecs_to_ticks(unsigned long usecs)
{
	unsigned long long ticks = (unsigned long long)usecs * TIMER_FREQ;
	do_div(ticks, 1000 * 1000);

	return ticks;
}

static inline void read_timer(void)
{
	struct timer_regs *timer_regs = (struct timer_regs *)TIMER_BASE;
	const unsigned long now = TIMER_MAX_VAL - readl(&timer_regs->timer3.value);

	if (now >= timer.last_read)
		timer.ticks += now - timer.last_read;
	else
		/* an overflow occurred */
		timer.ticks += TIMER_MAX_VAL - timer.last_read + now;

	timer.last_read = now;
}

/*
 * Get the number of ticks (in CONFIG_SYS_HZ resolution)
 */
unsigned long long get_ticks(void)
{
	unsigned long long sys_ticks;

	read_timer();

	sys_ticks = timer.ticks * CONFIG_SYS_HZ;
	do_div(sys_ticks, TIMER_FREQ);

	return sys_ticks;
}

unsigned long get_timer_masked(void)
{
	return get_ticks();
}

unsigned long get_timer(unsigned long base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked(void)
{
	read_timer();
	timer.ticks = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

void __udelay(unsigned long usec)
{
	unsigned long long target;

	read_timer();

	target = timer.ticks + usecs_to_ticks(usec);

	while (timer.ticks < target)
		read_timer();
}

int timer_init(void)
{
	struct timer_regs *timer_regs = (struct timer_regs *)TIMER_BASE;

	/* use timer 3 with 508KHz and free running, not enabled now */
	writel(TIMER_CLKSEL, &timer_regs->timer3.control);

	/* set initial timer value */
	writel(TIMER_MAX_VAL, &timer_regs->timer3.load);

	/* Enable the timer */
	writel(TIMER_ENABLE | TIMER_CLKSEL,
		&timer_regs->timer3.control);

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
