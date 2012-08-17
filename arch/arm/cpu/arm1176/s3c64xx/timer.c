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
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
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
#include <asm/arch/s3c6400.h>
#include <div64.h>

static ulong timer_load_val;

#define PRESCALER	167

static s3c64xx_timers *s3c64xx_get_base_timers(void)
{
	return (s3c64xx_timers *)ELFIN_TIMER_BASE;
}

/* macro to read the 16 bit timer */
static inline ulong read_timer(void)
{
	s3c64xx_timers *const timers = s3c64xx_get_base_timers();

	return timers->TCNTO4;
}

/* Internal tick units */
/* Last decremneter snapshot */
static unsigned long lastdec;
/* Monotonic incrementing timer */
static unsigned long long timestamp;

int timer_init(void)
{
	s3c64xx_timers *const timers = s3c64xx_get_base_timers();

	/* use PWM Timer 4 because it has no output */
	/*
	 * We use the following scheme for the timer:
	 * Prescaler is hard fixed at 167, divider at 1/4.
	 * This gives at PCLK frequency 66MHz approx. 10us ticks
	 * The timer is set to wrap after 100s, at 66MHz this obviously
	 * happens after 10,000,000 ticks. A long variable can thus
	 * keep values up to 40,000s, i.e., 11 hours. This should be
	 * enough for most uses:-) Possible optimizations: select a
	 * binary-friendly frequency, e.g., 1ms / 128. Also calculate
	 * the prescaler automatically for other PCLK frequencies.
	 */
	timers->TCFG0 = PRESCALER << 8;
	if (timer_load_val == 0) {
		timer_load_val = get_PCLK() / PRESCALER * (100 / 4); /* 100s */
		timers->TCFG1 = (timers->TCFG1 & ~0xf0000) | 0x20000;
	}

	/* load value for 10 ms timeout */
	lastdec = timers->TCNTB4 = timer_load_val;
	/* auto load, manual update of Timer 4 */
	timers->TCON = (timers->TCON & ~0x00700000) | TCON_4_AUTO |
		TCON_4_UPDATE;

	/* auto load, start Timer 4 */
	timers->TCON = (timers->TCON & ~0x00700000) | TCON_4_AUTO | COUNT_4_ON;
	timestamp = 0;

	return 0;
}

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

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = (usec + 9) / 10;
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)/* loop till event */
		 /*NOP*/;
}
