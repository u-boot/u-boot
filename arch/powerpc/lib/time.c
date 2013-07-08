/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

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
void __udelay(unsigned long usec)
{
	ulong ticks = usec2ticks (usec);
	wait_ticks (ticks);
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
	unsigned long temp;

#if defined(CONFIG_5xx) || defined(CONFIG_8xx)
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* unlock */
	immap->im_sitk.sitk_tbk = KAPWR_KEY;
#endif

	/* reset */
	asm volatile("li %0,0 ; mttbu %0 ; mttbl %0;"
	     : "=&r"(temp) );

#if defined(CONFIG_5xx) || defined(CONFIG_8xx)
	/* enable */
	immap->im_sit.sit_tbscr |= TBSCR_TBE;
#endif
	return (0);
}
/* ------------------------------------------------------------------------- */
