/*
 * (C) Copyright 2004
 * Texas Instruments
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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
#include <asm/arch/bits.h>
#include <asm/arch/omap2420.h>

#define TIMER_LOAD_VAL 0

/* macro to read the 32 bit timer */
#define READ_TIMER (*volatile ulong*(CFG_TIMERBASE+TCRR))

static ulong timestamp;
static ulong lastinc;

/*
 * nothing really to do with interrupts, just starts up a counter.
 */
int interrupt_init(void)
{
	int32_t val;

	/* Start the counter ticking up */
	/* reload value on overflow*/
	*((int32_t *) (CFG_TIMERBASE + TLDR)) = TIMER_LOAD_VAL;
	/* mask to enable timer*/
	val = (CFG_PVT << 2) | BIT5 | BIT1 | BIT0;
	*((int32_t *) (CFG_TIMERBASE + TCLR)) = val;	/* start timer */

	reset_timer_masked(); /* init the timestamp and lastinc value */

	return(0);
}
/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

/* delay x useconds AND perserve advance timstamp value */
void udelay(unsigned long usec)
{
	ulong tmo, tmp;

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CFG_HZ;
		/* finish normalize. */
		tmo /= 1000;
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CFG_HZ;
		tmo /= (1000*1000);
	}
	/* get current timestamp */
	tmp = get_timer(0);
	if ((tmo + tmp + 1) < tmp)
		/* setting this forward will roll time stamp */
		/* reset "advancing" timestamp to 0, set lastinc value */
		reset_timer_masked();
	else
		/* else, set advancing stamp wake up time */
		tmo	+= tmp;
	while (get_timer_masked() < tmo)/* loop till event */
		/*NOP*/;
}

void reset_timer_masked(void)
{
	/* reset time */
	/* capture current incrementer value time */
	lastinc = READ_TIMER;
	/* start "advancing" time stamp from 0 */
	timestamp = 0;
}

ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;	/* current tick value */

	/* normal mode (non roll) */
	if (now >= lastinc)
		/* move stamp forward with absolute diff ticks */
		timestamp += (now - lastinc);
	else
		/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	lastinc = now;
	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {
		/* "big" number, spread normalization to seconds */
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CFG_HZ;
		tmo /= 1000;/* finish normalize. */
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CFG_HZ;
		tmo /= (1000*1000);
	}
	endtime = get_timer_masked() + tmo;

	do {
		ulong now = get_timer_masked();
		diff = endtime - now;
	} while (diff >= 0);
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
ulong get_tbclk(void)
{
	ulong tbclk;
	tbclk = CFG_HZ;
	return tbclk;
}
