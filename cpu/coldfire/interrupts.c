/*
 * (C) Copyright 2000-2003
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
#include <watchdog.h>
#ifdef CONFIG_M5272
#include <asm/m5272.h>
#endif
#ifdef CONFIG_M5282
#include <asm/m5282.h>
#endif
#include <asm/mcftimer.h>
#include <asm/processor.h>
#include <commproc.h>

static ulong timestamp;
static unsigned short lastinc;

void coldfire_timer_init (void)
{
	volatile unsigned short *timerp;

	timerp = (volatile unsigned short *) (MCF_MBAR + MCFTIMER_BASE4);
	timestamp = 0;

#ifdef CONFIG_M5272
	/* Set up TIMER 4 as poll clock */
	timerp[MCFTIMER_TMR] = MCFTIMER_TMR_DISABLE;
	timerp[MCFTIMER_TRR] = lastinc = 0;
	timerp[MCFTIMER_TMR] = (65 << 8) | MCFTIMER_TMR_CLK1 |
		MCFTIMER_TMR_FREERUN | MCFTIMER_TMR_ENABLE;
#endif

#ifdef CONFIG_M5282
	/* Set up TIMER 4 as poll clock */
	timerp[MCFTIMER_PCSR] = MCFTIMER_PCSR_OVW;
	timerp[MCFTIMER_PMR] = lastinc = 0;
	timerp[MCFTIMER_PCSR] =
		(5 << 8) | MCFTIMER_PCSR_EN | MCFTIMER_PCSR_OVW;
#endif
}


void irq_install_handler (int vec, interrupt_handler_t * handler, void *arg)
{
}
void irq_free_handler (int vec)
{
}

int interrupt_init (void)
{
	coldfire_timer_init ();
	return 0;
}

void enable_interrupts ()
{
}
int disable_interrupts ()
{
	return 0;
}

void set_timer (ulong t)
{
	volatile unsigned short *timerp;

	timerp = (volatile unsigned short *) (MCF_MBAR + MCFTIMER_BASE4);
	timestamp = 0;

#ifdef CONFIG_M5272
	timerp[MCFTIMER_TRR] = lastinc = 0;
#endif

#ifdef CONFIG_M5282
	timerp[MCFTIMER_PMR] = lastinc = 0;
#endif
}

ulong get_timer (ulong base)
{
	unsigned short now, diff;
	volatile unsigned short *timerp;

	timerp = (volatile unsigned short *) (MCF_MBAR + MCFTIMER_BASE4);

#ifdef CONFIG_M5272
	now = timerp[MCFTIMER_TCN];
	diff = (now - lastinc);
#endif

#ifdef CONFIG_M5282
	now = timerp[MCFTIMER_PCNTR];
	diff = -(now - lastinc);
#endif

	timestamp += diff;
	lastinc = now;
	return timestamp - base;
}

void wait_ticks (unsigned long ticks)
{
	set_timer (0);
	while (get_timer (0) < ticks);
}
