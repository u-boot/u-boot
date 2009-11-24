/*
 * (C) Copyright 2004-2005, Greg Ungerer <greg.ungerer@opengear.com>
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
#include <asm/arch/platform.h>

/*
 * Handy KS8695 register access functions.
 */
#define	ks8695_read(a)    *((volatile ulong *) (KS8695_IO_BASE + (a)))
#define	ks8695_write(a,v) *((volatile ulong *) (KS8695_IO_BASE + (a))) = (v)

ulong timer_ticks;

int timer_init (void)
{
	reset_timer();

	return 0;
}

/*
 * Initial timer set constants. Nothing complicated, just set for a 1ms
 * tick.
 */
#define	TIMER_INTERVAL	(TICKS_PER_uSEC * mSEC_1)
#define	TIMER_COUNT	(TIMER_INTERVAL / 2)
#define	TIMER_PULSE	TIMER_COUNT

void reset_timer_masked(void)
{
	/* Set the hadware timer for 1ms */
	ks8695_write(KS8695_TIMER1, TIMER_COUNT);
	ks8695_write(KS8695_TIMER1_PCOUNT, TIMER_PULSE);
	ks8695_write(KS8695_TIMER_CTRL, 0x2);
	timer_ticks = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer_masked(void)
{
	/* Check for timer wrap */
	if (ks8695_read(KS8695_INT_STATUS) & KS8695_INTMASK_TIMERINT1) {
		/* Clear interrupt condition */
		ks8695_write(KS8695_INT_STATUS, KS8695_INTMASK_TIMERINT1);
		timer_ticks++;
	}
	return timer_ticks;
}

ulong get_timer(ulong base)
{
       return (get_timer_masked() - base);
}

void set_timer(ulong t)
{
	timer_ticks = t;
}

void __udelay(ulong usec)
{
	ulong start = get_timer_masked();
	ulong end;

	/* Only 1ms resolution :-( */
	end = usec / 1000;
	while (get_timer(start) < end)
		;
}

void reset_cpu (ulong ignored)
{
	ulong tc;

	/* Set timer0 to watchdog, and let it timeout */
	tc = ks8695_read(KS8695_TIMER_CTRL) & 0x2;
	ks8695_write(KS8695_TIMER_CTRL, tc);
	ks8695_write(KS8695_TIMER0, ((10 << 8) | 0xff));
	ks8695_write(KS8695_TIMER_CTRL, (tc | 0x1));

	/* Should only wait here till watchdog resets */
	for (;;)
		;
}
