/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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

/* Implemented by SPARC CPUs */
extern void cpu_wait_ticks(unsigned long ticks);
extern unsigned long cpu_usec2ticks(unsigned long usec);
extern unsigned long cpu_ticks2usec(unsigned long ticks);

/* ------------------------------------------------------------------------- */

void wait_ticks(unsigned long ticks)
{
	cpu_wait_ticks(ticks);
}

/*
 * This function is intended for SHORT delays only.
 */
unsigned long usec2ticks(unsigned long usec)
{
	return cpu_usec2ticks(usec);
}

/* ------------------------------------------------------------------------- */

/*
 * We implement the delay by converting the delay (the number of
 * microseconds to wait) into a number of time base ticks; then we
 * watch the time base until it has incremented by that amount.
 */
void __udelay(unsigned long usec)
{
	ulong ticks = usec2ticks(usec);

	wait_ticks(ticks);
}

/* ------------------------------------------------------------------------- */

unsigned long ticks2usec(unsigned long ticks)
{
	return cpu_ticks2usec(ticks);
}

/* ------------------------------------------------------------------------- */

int init_timebase(void)
{

	return (0);
}

/* ------------------------------------------------------------------------- */
