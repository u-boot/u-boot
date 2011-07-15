/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2010
 * Matthias Weisser, Graf-Syteco <weisserm@arcor.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <div64.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

#define TIMER_LOAD_VAL	0xffffffff
#define TIMER_FREQ	(CONFIG_MB86R0x_IOCLK  / 256)

DECLARE_GLOBAL_DATA_PTR;

#define timestamp gd->tbl
#define lastdec gd->lastinc

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, TIMER_FREQ);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= TIMER_FREQ;
	do_div(usec, 1000000);

	return usec;
}

/* nothing really to do with interrupts, just starts up a counter. */
int timer_init(void)
{
	struct mb86r0x_timer * timer = (struct mb86r0x_timer *)
					MB86R0x_TIMER_BASE;
	ulong ctrl = readl(&timer->control);

	writel(TIMER_LOAD_VAL, &timer->load);

	ctrl |= MB86R0x_TIMER_ENABLE | MB86R0x_TIMER_PRS_8S |
		MB86R0x_TIMER_SIZE_32;

	writel(ctrl, &timer->control);

	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */
unsigned long long get_ticks(void)
{
	struct mb86r0x_timer * timer = (struct mb86r0x_timer *)
					MB86R0x_TIMER_BASE;
	ulong now = readl(&timer->value);

	if (now <= lastdec) {
		/* normal mode (non roll) */
		/* move stamp forward with absolut diff ticks */
		timestamp += lastdec - now;
	} else {
		/* we have rollover of incrementer */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;
	return timestamp;
}

void reset_timer_masked(void)
{
	struct mb86r0x_timer * timer = (struct mb86r0x_timer *)
					MB86R0x_TIMER_BASE;

	/* capture current value time */
	lastdec = readl(&timer->value);
	timestamp = 0; /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	return tick_to_time(get_ticks());
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = usec_to_tick(usec);
	tmp = get_ticks();			/* get current timestamp */

	while ((get_ticks() - tmp) < tmo)	/* loop till event */
		 /*NOP*/;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = TIMER_FREQ;
	return tbclk;
}
