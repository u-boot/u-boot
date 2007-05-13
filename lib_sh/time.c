/*
 * Copyright (c) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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
#include <asm/processer.h>

static void tmu_timer_start (unsigned int timer)
{
	if (timer > 2)
		return;

	*((volatile unsigned char *) TSTR0) |= (1 << timer);
}

int timer_init (void)
{
	*(volatile u16 *)TCR0 = 0;

	tmu_timer_start (0);
	return 0;
}

unsigned long long get_ticks (void)
{
	return (0 - *((volatile unsigned int *) TCNT0));
}

unsigned long get_timer (unsigned long base)
{
	return ((0 - *((volatile unsigned int *) TCNT0)) - base);
}

void set_timer (unsigned long t)
{
	*((volatile unsigned int *) TCNT0) = (0 - t);
}

void reset_timer (void)
{
	set_timer (0);
}

void udelay (unsigned long usec)
{
	unsigned int start = get_timer (0);
	unsigned int end = start + (usec * ((CFG_HZ + 500000) / 1000000));

	while (get_timer (0) < end)
		continue;
}

unsigned long get_tbclk (void)
{
	return CFG_HZ;
}

