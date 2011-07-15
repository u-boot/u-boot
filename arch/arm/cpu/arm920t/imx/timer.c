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
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
#if defined (CONFIG_IMX)

#include <asm/arch/imx-regs.h>

int timer_init (void)
{
	int i;
	/* setup GP Timer 1 */
	TCTL1 = TCTL_SWR;
	for ( i=0; i<100; i++) TCTL1 = 0; /* We have no udelay by now */
	TPRER1 = get_PERCLK1() / 1000000; /* 1 MHz */
	TCTL1 |= TCTL_FRR | (1<<1); /* Freerun Mode, PERCLK1 input */

	reset_timer_masked();

	return (0);
}

/*
 * timer without interrupts
 */
ulong get_timer (ulong base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked (void)
{
	TCTL1 &= ~TCTL_TEN;
	TCTL1 |= TCTL_TEN; /* Enable timer */
}

ulong get_timer_masked (void)
{
	return TCN1;
}

void udelay_masked (unsigned long usec)
{
	ulong endtime = get_timer_masked() + usec;
	signed long diff;

	do {
		ulong now = get_timer_masked ();
		diff = endtime - now;
	} while (diff >= 0);
}

void __udelay (unsigned long usec)
{
	udelay_masked(usec);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;

	return tbclk;
}

/*
 * Reset the cpu by setting up the watchdog timer and let him time out
 */
void reset_cpu (ulong ignored)
{
	/* Disable watchdog and set Time-Out field to 0 */
	WCR = 0x00000000;

	/* Write Service Sequence */
	WSR = 0x00005555;
	WSR = 0x0000AAAA;

	/* Enable watchdog */
	WCR = 0x00000001;

	while (1);
	/*NOTREACHED*/
}

#endif /* defined (CONFIG_IMX) */
