/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>


extern void dly_clks( unsigned long ticks );

void __udelay(unsigned long usec)
{
	/* The Nios core doesn't have a timebase, so we do our
	 * best for now and call a low-level loop that counts
	 * cpu clocks.
	 */
	unsigned long cnt = (CONFIG_SYS_CLK_FREQ/1000000) * usec;
	dly_clks (cnt);
}
