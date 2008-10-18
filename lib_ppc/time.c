/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef CONFIG_WD_PERIOD
# define CONFIG_WD_PERIOD	(10 * 1000 * 1000)	/* 10 seconds default*/
#endif

/* ------------------------------------------------------------------------- */

/*
 * This function is intended for SHORT delays only.
 * It will overflow at around 10 seconds @ 400MHz,
 * or 20 seconds @ 200MHz.
 */
unsigned long usec2ticks(unsigned long usec)
{
	ulong ticks;

	if (usec < 1000) {
		ticks = ((usec * (get_tbclk()/1000)) + 500) / 1000;
	} else {
		ticks = ((usec / 10) * (get_tbclk() / 100000));
	}

	return (ticks);
}

/* ------------------------------------------------------------------------- */

/*
 * We implement the delay by converting the delay (the number of
 * microseconds to wait) into a number of time base ticks; then we
 * watch the time base until it has incremented by that amount.
 */
void udelay(unsigned long usec)
{
	ulong ticks, kv;

	do {
		kv = usec > CONFIG_WD_PERIOD ? CONFIG_WD_PERIOD : usec;
		ticks = usec2ticks (kv);
		wait_ticks (ticks);
		usec -= kv;
	} while(usec);
}

/* ------------------------------------------------------------------------- */
#ifndef CONFIG_NAND_SPL
unsigned long ticks2usec(unsigned long ticks)
{
	ulong tbclk = get_tbclk();

	/* usec = ticks * 1000000 / tbclk
	 * Multiplication would overflow at ~4.2e3 ticks,
	 * so we break it up into
	 * usec = ( ( ticks * 1000) / tbclk ) * 1000;
	 */
	ticks *= 1000L;
	ticks /= tbclk;
	ticks *= 1000L;

	return ((ulong)ticks);
}
#endif
/* ------------------------------------------------------------------------- */

int init_timebase (void)
{
#if defined(CONFIG_5xx) || defined(CONFIG_8xx)
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* unlock */
	immap->im_sitk.sitk_tbk = KAPWR_KEY;
#endif

	/* reset */
	asm ("li 3,0 ; mttbu 3 ; mttbl 3 ;");

#if defined(CONFIG_5xx) || defined(CONFIG_8xx)
	/* enable */
	immap->im_sit.sit_tbscr |= TBSCR_TBE;
#endif
	return (0);
}
/* ------------------------------------------------------------------------- */
