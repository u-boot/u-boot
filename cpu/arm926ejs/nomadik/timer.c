/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
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
#include <arm926ejs.h>

#define TIMER_LOAD_VAL 0xffffffff

/* macro to read the 32 bit timer */
#define READ_TIMER readl(CONFIG_SYS_TIMERBASE + 20)

static ulong timestamp;
static ulong lastdec;

/* nothing really to do with interrupts, just starts up a counter. */
int timer_init(void)
{
	/* Load timer with initial value */
	writel(TIMER_LOAD_VAL, CONFIG_SYS_TIMERBASE + 16);

	/*
	 * Set timer to be enabled, free-running, no interrupts, 256 divider,
	 * 32-bit, wrap-mode
	 */
	writel(0x8a, CONFIG_SYS_TIMERBASE + 24);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
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

	if (usec >= 1000) {
		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize */
		tmo *= CONFIG_SYS_HZ;	/* find number of "ticks" */
		tmo /= 1000;		/* finish normalize. */
	} else {
		/* small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000 * 1000);
	}

	tmp = get_timer(0);		/* get current timestamp */
	if ((tmo + tmp + 1) < tmp)	/* will roll time stamp? */
		reset_timer_masked();	/* reset to 0, set lastdec value */
	else
		tmo += tmp;

	while (get_timer_masked() < tmo)
		/* nothing */ ;
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER;	/* capure current decrementer value time */
	timestamp = 0;		/* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;		/* current tick value */

	if (lastdec >= now) {		/* normal mode (non roll) */
		/* move stamp fordward */
		timestamp += lastdec - now;
	} else {
		/*
		 * An overflow is expected.
		 * nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
{
	ulong tmo;

	if (usec >= 1000) {
		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize */
		tmo *= CONFIG_SYS_HZ;	/* find number of "ticks" */
		tmo /= 1000;		/* finish normalize. */
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}

	reset_timer_masked();
	/* set "advancing" timestamp to 0, set lastdec vaule */

	while (get_timer_masked() < tmo)
		/* nothing */ ;
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

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}
