/*
 * (C) Copyright 2002
 * Lineo, Inc. <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
/*#include <asm/io.h>*/
#include <asm/arch/hardware.h>
/*#include <asm/proc/ptrace.h>*/

/* the number of clocks per CONFIG_SYS_HZ */
#define TIMER_LOAD_VAL (CONFIG_SYS_HZ_CLOCK/CONFIG_SYS_HZ)

/* macro to read the 16 bit timer */
#define READ_TIMER (tmr->TC_CV & 0x0000ffff)
AT91PS_TC tmr;

static ulong timestamp;
static ulong lastinc;

int timer_init (void)
{
	tmr = AT91C_BASE_TC0;

	/* enables TC1.0 clock */
	*AT91C_PMC_PCER = 1 << AT91C_ID_TC0;  /* enable clock */

	*AT91C_TCB0_BCR = 0;
	*AT91C_TCB0_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_NONE | AT91C_TCB_TC2XC2S_NONE;
	tmr->TC_CCR = AT91C_TC_CLKDIS;
#define AT91C_TC_CMR_CPCTRG (1 << 14)
	/* set to MCLK/2 and restart the timer when the vlaue in TC_RC is reached */
	tmr->TC_CMR = AT91C_TC_TIMER_DIV1_CLOCK | AT91C_TC_CMR_CPCTRG;

	tmr->TC_IDR = ~0ul;
	tmr->TC_RC = TIMER_LOAD_VAL;
	lastinc = 0;
	tmr->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN;
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
	return get_timer_masked () - base;
}

void __udelay (unsigned long usec)
{
	udelay_masked(usec);
}

void reset_timer_masked (void)
{
	/* reset time */
	lastinc = READ_TIMER;
	timestamp = 0;
}

ulong get_timer_raw (void)
{
	ulong now = READ_TIMER;

	if (now >= lastinc) {
		/* normal mode */
		timestamp += now - lastinc;
	} else {
		/* we have an overflow ... */
		timestamp += now + TIMER_LOAD_VAL - lastinc;
	}
	lastinc = now;

	return timestamp;
}

ulong get_timer_masked (void)
{
	return get_timer_raw()/TIMER_LOAD_VAL;
}

void udelay_masked (unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	tmo = CONFIG_SYS_HZ_CLOCK / 1000;
	tmo *= usec;
	tmo /= 1000;

	endtime = get_timer_raw () + tmo;

	do {
		ulong now = get_timer_raw ();
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

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}
