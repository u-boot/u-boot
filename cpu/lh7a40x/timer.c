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
#include <lh7a40x.h>

static ulong timer_load_val = 0;

/* macro to read the 16 bit timer */
static inline ulong READ_TIMER(void)
{
	lh7a40x_timers_t* timers = LH7A40X_TIMERS_PTR;
	lh7a40x_timer_t* timer = &timers->timer1;

	return (timer->value & 0x0000ffff);
}

static ulong timestamp;
static ulong lastdec;

int timer_init (void)
{
	lh7a40x_timers_t* timers = LH7A40X_TIMERS_PTR;
	lh7a40x_timer_t* timer = &timers->timer1;

	/* a periodic timer using the 508kHz source */
	timer->control = (TIMER_PER | TIMER_CLK508K);

	if (timer_load_val == 0) {
		/*
		 * 10ms period with 508.469kHz clock = 5084
		 */
		timer_load_val = CONFIG_SYS_HZ/100;
	}

	/* load value for 10 ms timeout */
	lastdec = timer->load = timer_load_val;

	/* auto load, start timer */
	timer->control = timer->control | TIMER_EN;
	timestamp = 0;

	return (0);
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
	return (get_timer_masked() - base);
}

void set_timer (ulong t)
{
	timestamp = t;
}

void __udelay (unsigned long usec)
{
	ulong tmo,tmp;

	/* normalize */
	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	}
	else {
		if (usec > 1) {
			tmo = usec * CONFIG_SYS_HZ;
			tmo /= (1000*1000);
		}
		else
			tmo = 1;
	}

	/* check for rollover during this delay */
	tmp = get_timer (0);
	if ((tmp + tmo) < tmp )
		reset_timer_masked();  /* timer would roll over */
	else
		tmo += tmp;

	while (get_timer_masked () < tmo);
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER();
	timestamp = 0;
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER();

	if (lastdec >= now) {
		/* normal mode */
		timestamp += (lastdec - now);
	} else {
		/* we have an overflow ... */
		timestamp += ((lastdec + timer_load_val) - now);
	}
	lastdec = now;

	return timestamp;
}

void udelay_masked (unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	/* normalize */
	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	} else {
		if (usec > 1) {
			tmo = usec * CONFIG_SYS_HZ;
			tmo /= (1000*1000);
		} else {
			tmo = 1;
		}
	}

	endtime = get_timer_masked () + tmo;

	do {
		ulong now = get_timer_masked ();
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
ulong get_tbclk (void)
{
	ulong tbclk;

	tbclk = timer_load_val * 100;

	return tbclk;
}
