/*
 * (C) Copyright 2009
 * 2N Telekomunikace, <www.2n.cz>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <arm925t.h>
#include <configs/omap1510.h>
#include <asm/io.h>

#define TIMER_LOAD_VAL	0xffffffff
#define TIMER_CLOCK	(CONFIG_SYS_CLK_FREQ / (2 << CONFIG_SYS_PTV))

static uint32_t timestamp;
static uint32_t lastdec;

/* nothing really to do with interrupts, just starts up a counter. */
int timer_init (void)
{
	/* Start the decrementer ticking down from 0xffffffff */
	__raw_writel(TIMER_LOAD_VAL, CONFIG_SYS_TIMERBASE + LOAD_TIM);
	__raw_writel(MPUTIM_ST | MPUTIM_AR | MPUTIM_CLOCK_ENABLE |
		(CONFIG_SYS_PTV << MPUTIM_PTV_BIT),
		CONFIG_SYS_TIMERBASE + CNTL_TIMER);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */

void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void set_timer (ulong t)
{
	timestamp = t;
}

/* delay x useconds AND preserve advance timestamp value */
void udelay (unsigned long usec)
{
	int32_t tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	uint32_t now, last = __raw_readl(CONFIG_SYS_TIMERBASE + READ_TIM);

	while (tmo > 0) {
		now = __raw_readl(CONFIG_SYS_TIMERBASE + READ_TIM);
		if (last < now) /* count down timer underflow */
			tmo -= TIMER_LOAD_VAL - now + last;
		else
			tmo -= last - now;
		last = now;
	}
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = __raw_readl(CONFIG_SYS_TIMERBASE + READ_TIM) /
			(TIMER_CLOCK / CONFIG_SYS_HZ);
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked (void)
{
	uint32_t now = __raw_readl(CONFIG_SYS_TIMERBASE + READ_TIM) /
			(TIMER_CLOCK / CONFIG_SYS_HZ);
	if (lastdec < now)	/* count down timer underflow */
		timestamp += TIMER_LOAD_VAL / (TIMER_CLOCK / CONFIG_SYS_HZ) -
				now + lastdec;
	else
		timestamp += lastdec - now;
	lastdec = now;

	return timestamp;
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
	return CONFIG_SYS_HZ;
}
