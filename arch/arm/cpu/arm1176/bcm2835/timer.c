/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>

int timer_init(void)
{
	return 0;
}

ulong get_timer(ulong base)
{
	struct bcm2835_timer_regs *regs =
		(struct bcm2835_timer_regs *)BCM2835_TIMER_PHYSADDR;

	return readl(&regs->clo) - base;
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

void __udelay(unsigned long usec)
{
	ulong endtime;
	signed long diff;

	endtime = get_timer(0) + usec;

	do {
		ulong now = get_timer(0);
		diff = endtime - now;
	} while (diff >= 0);
}
