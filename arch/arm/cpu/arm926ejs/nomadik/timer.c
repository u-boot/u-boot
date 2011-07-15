/*
 * (C) Copyright 2009 Alessandro Rubini
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
#include <asm/io.h>
#include <asm/arch/mtu.h>

/*
 * The timer is a decrementer, we'll left it free running at 2.4MHz.
 * We have 2.4 ticks per microsecond and an overflow in almost 30min
 */
#define TIMER_CLOCK		(24 * 100 * 1000)
#define COUNT_TO_USEC(x)	((x) * 5 / 12)	/* overflows at 6min */
#define USEC_TO_COUNT(x)	((x) * 12 / 5)	/* overflows at 6min */
#define TICKS_PER_HZ		(TIMER_CLOCK / CONFIG_SYS_HZ)
#define TICKS_TO_HZ(x)		((x) / TICKS_PER_HZ)

/* macro to read the decrementing 32 bit timer as an increasing count */
#define READ_TIMER() (0 - readl(CONFIG_SYS_TIMERBASE + MTU_VAL(0)))

/* Configure a free-running, auto-wrap counter with no prescaler */
int timer_init(void)
{
	ulong val;

	writel(MTU_CRn_ENA | MTU_CRn_PRESCALE_1 | MTU_CRn_32BITS,
	       CONFIG_SYS_TIMERBASE + MTU_CR(0));

	/* Reset the timer */
	writel(0, CONFIG_SYS_TIMERBASE + MTU_LR(0));
	/*
	 * The load-register isn't really immediate: it changes on clock
	 * edges, so we must wait for our newly-written value to appear.
	 * Since we might miss reading 0, wait for any change in value.
	 */
	val = READ_TIMER();
	while (READ_TIMER() == val)
		;

	return 0;
}

/* Return how many HZ passed since "base" */
ulong get_timer(ulong base)
{
	return  TICKS_TO_HZ(READ_TIMER()) - base;
}

/* Delay x useconds */
void __udelay(unsigned long usec)
{
	ulong ini, end;

	ini = READ_TIMER();
	end = ini + USEC_TO_COUNT(usec);
	while ((signed)(end - READ_TIMER()) > 0)
		;
}
