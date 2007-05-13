/*
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
#include <asm/processor.h>

#define TMU_MAX_COUNTER (~0UL)

static void tmu_timer_start (unsigned int timer)
{
	if (timer > 2)
		return;

	*((volatile unsigned char *) TSTR0) |= (1 << timer);
}

int timer_init (void)
{
	/* Divide clock by 4 */
	*(volatile u16 *)TCR0 = 0;

	tmu_timer_start (0);
	return 0;
}

/*
   In theory we should return a true 64bit value (ie something that doesn't
   overflow). However, we don't. Therefore if TMU runs at fastest rate of
   6.75 MHz this value will wrap after u-boot has been running for approx
   10 minutes.
*/
unsigned long long get_ticks (void)
{
	return (0 - *((volatile unsigned int *) TCNT0));
}

unsigned long get_timer (unsigned long base)
{
	unsigned long n = 
		*((volatile unsigned int *)TCNT0) ;
	
	return ((int)n - base ) < 0 ? ( TMU_MAX_COUNTER - ( base -n )):(n - base );
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
	unsigned int end = 0;
	if (usec > 1000000)
		end = ((usec/100000) * CFG_HZ) / 10;
	else if (usec > 1000)
		end = ((usec/100) * CFG_HZ) / 10000;
	else
		end = (usec * CFG_HZ) / 1000000;

	while (get_timer (0) < end)
		continue;
}

unsigned long get_tbclk (void)
{
	return CFG_HZ;
}

