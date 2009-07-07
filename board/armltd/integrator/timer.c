/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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
#include <div64.h>

#ifdef CONFIG_ARCH_CINTEGRATOR
#define DIV_CLOCK_INIT	1
#define TIMER_LOAD_VAL	0xFFFFFFFFL
#else
#define DIV_CLOCK_INIT	256
#define TIMER_LOAD_VAL	0x0000FFFFL
#endif
/* The Integrator/CP timer1 is clocked at 1MHz
 * can be divided by 16 or 256
 * and can be set up as a 32-bit timer
 */
/* U-Boot expects a 32 bit timer, running at CONFIG_SYS_HZ */
/* Keep total timer count to avoid losing decrements < div_timer */
static unsigned long long total_count = 0;
static unsigned long long lastdec;	 /* Timer reading at last call	   */
/* Divisor applied to timer clock */
static unsigned long long div_clock = DIV_CLOCK_INIT;
static unsigned long long div_timer = 1; /* Divisor to convert timer reading
					  * change to U-Boot ticks
					  */
/* CONFIG_SYS_HZ = CONFIG_SYS_HZ_CLOCK/(div_clock * div_timer) */
static ulong timestamp;		/* U-Boot ticks since startup */

#define READ_TIMER (*(volatile ulong *)(CONFIG_SYS_TIMERBASE+4))

/* all function return values in U-Boot ticks i.e. (1/CONFIG_SYS_HZ) sec
 *  - unless otherwise stated
 */

/* starts up a counter
 * - the Integrator/CP timer can be set up to issue an interrupt */
int timer_init (void)
{
	/* Load timer with initial value */
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 0) = TIMER_LOAD_VAL;
#ifdef CONFIG_ARCH_CINTEGRATOR
	/* Set timer to be
	 *	enabled		 1
	 *	periodic	 1
	 *	no interrupts	 0
	 *	X		 0
	 *	divider 1	00 == less rounding error
	 *	32 bit		 1
	 *	wrapping	 0
	 */
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = 0x000000C2;
#else
	/* Set timer to be
	 *	enabled		 1
	 *	free-running	 0
	 *	XX		00
	 *	divider 256	10
	 *	XX		00
	 */
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = 0x00000088;
#endif

	/* init the timestamp */
	total_count = 0ULL;
	reset_timer_masked();

	div_timer = CONFIG_SYS_HZ_CLOCK;
	do_div(div_timer, CONFIG_SYS_HZ);
	do_div(div_timer, div_clock);

	return (0);
}

/*
 * timer without interrupts
 */
void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base_ticks)
{
	return get_timer_masked () - base_ticks;
}

void set_timer (ulong ticks)
{
	timestamp   = ticks;
	total_count = ticks * div_timer;
}

/* delay usec useconds */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	/* Convert to U-Boot ticks */
	tmo  = usec * CONFIG_SYS_HZ;
	tmo /= (1000000L);

	tmp  = get_timer_masked();	/* get current timestamp */
	tmo += tmp;			/* form target timestamp */

	while (get_timer_masked () < tmo) {/* loop till event */
		/*NOP*/;
	}
}

void reset_timer_masked (void)
{
	/* capure current decrementer value    */
	lastdec	  = READ_TIMER;
	/* start "advancing" time stamp from 0 */
	timestamp = 0L;
}

/* converts the timer reading to U-Boot ticks	       */
/* the timestamp is the number of ticks since reset    */
ulong get_timer_masked (void)
{
	/* get current count */
	unsigned long long now = READ_TIMER;

	if(now > lastdec) {
		/* Must have wrapped */
		total_count += lastdec + TIMER_LOAD_VAL + 1 - now;
	} else {
		total_count += lastdec - now;
	}
	lastdec	= now;

	/* Reuse "now" */
	now = total_count;
	do_div(now, div_timer);
	timestamp = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	udelay(usec);
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
 * Return the timebase clock frequency
 * i.e. how often the timer decrements
 */
ulong get_tbclk (void)
{
	unsigned long long tmp = CONFIG_SYS_HZ_CLOCK;

	do_div(tmp, div_clock);

	return tmp;
}
