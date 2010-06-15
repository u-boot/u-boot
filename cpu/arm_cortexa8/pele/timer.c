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
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * And thoroughly mangled into dragonfire form, probably never to escape Xilinx.
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
#include <asm/proc-armv/ptrace.h>
#include <div64.h>

static ulong timer_load_val;

#define PRESCALER	167

/* read the 16 bit timer */
static inline ulong read_timer(void)
{	
#ifdef NOTNOW_BHILL
	return xdfttc_readl(COUNT_VALUE);
#endif
}

/* Internal tick units */
/* Last decremneter snapshot */
static unsigned long lastdec;
/* Monotonic incrementing timer */
static unsigned long long timestamp;

int timer_init()
{
	reset_timer_masked();
}

#ifdef NOTNOW_BHILL

int interrupt_init(void)
{
	/* complete garbage. */
	timer_load_val = 0x800000;
	lastdec = 0x100;

	return 0;
}
#endif

/*
 * timer without interrupts
 */

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	ulong now = read_timer();

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec + timer_load_val - now;
	}
	lastdec = now;

	return timestamp;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	/* We overrun in 100s */
	return (ulong)(timer_load_val / 100);
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = read_timer();
	timestamp = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer_masked(void)
{
	unsigned long long res = get_ticks();
	do_div (res, (timer_load_val / (100 * CONFIG_SYS_HZ)));
	return res;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t * (timer_load_val / (100 * CONFIG_SYS_HZ));
}

void udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = (usec + 9) / 10;
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)/* loop till event */
		 /*NOP*/;
}
