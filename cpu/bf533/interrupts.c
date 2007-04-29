/*
 * U-boot - interrupts.c Interrupt related routines
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on interrupts.c
 * Copyright 1996 Roman Zippel
 * Copyright 1999 D. Jeff Dionne <jeff@uclinux.org>
 * Copyright 2000-2001 Lineo, Inc. D. Jefff Dionne <jeff@lineo.ca>
 * Copyright 2002 Arcturus Networks Inc. MaTed <mated@sympatico.ca>
 * Copyright 2003 Metrowerks/Motorola
 * Copyright 2003 Bas Vermeulen <bas@buyways.nl>,
 *			BuyWays B.V. (www.buyways.nl)
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.

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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/machdep.h>
#include <asm/irq.h>
#include <config.h>
#include <asm/blackfin.h>
#include "cpu.h"

static ulong timestamp;
static ulong last_time;
static int int_flag;

int irq_flags;			/* needed by asm-blackfin/system.h */

/* Functions just to satisfy the linker */

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On BF533 it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On BF533 it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CFG_HZ;
	return tbclk;
}

void enable_interrupts(void)
{
	restore_flags(int_flag);
}

int disable_interrupts(void)
{
	save_and_cli(int_flag);
	return 1;
}

int interrupt_init(void)
{
	return (0);
}

void udelay(unsigned long usec)
{
	unsigned long delay, start, stop;
	unsigned long cclk;
	cclk = (CONFIG_CCLK_HZ);

	while (usec > 1) {
		/*
		 * how many clock ticks to delay?
		 *  - request(in useconds) * clock_ticks(Hz) / useconds/second
		 */
		if (usec < 1000) {
			delay = (usec * (cclk / 244)) >> 12;
			usec = 0;
		} else {
			delay = (1000 * (cclk / 244)) >> 12;
			usec -= 1000;
		}

		asm volatile (" %0 = CYCLES;":"=r" (start));
		do {
			asm volatile (" %0 = CYCLES; ":"=r" (stop));
		} while (stop - start < delay);
	}

	return;
}

void timer_init(void)
{
	*pTCNTL = 0x1;
	*pTSCALE = 0x0;
	*pTCOUNT = MAX_TIM_LOAD;
	*pTPERIOD = MAX_TIM_LOAD;
	*pTCNTL = 0x7;
	asm("CSYNC;");

	timestamp = 0;
	last_time = 0;
}

/* Any network command or flash
 * command is started get_timer shall
 * be called before TCOUNT gets reset,
 * to implement the accurate timeouts.
 *
 * How ever milliconds doesn't return
 * the number that has been elapsed from
 * the last reset.
 *
 *  As get_timer is used in the u-boot
 *  only for timeouts this should be
 *  sufficient
 */
ulong get_timer(ulong base)
{
	ulong milisec;

	/* Number of clocks elapsed */
	ulong clocks = (MAX_TIM_LOAD - (*pTCOUNT));

	/**
	 * Find if the TCOUNT is reset
	 * timestamp gives the number of times
	 * TCOUNT got reset
	 */
	if (clocks < last_time)
		timestamp++;
	last_time = clocks;

	/* Get the number of milliseconds */
	milisec = clocks / (CONFIG_CCLK_HZ / 1000);

	/**
	 * Find the number of millisonds
	 * that got elapsed before this TCOUNT cycle
	 */
	milisec += timestamp * (MAX_TIM_LOAD / (CONFIG_CCLK_HZ / 1000));

	return (milisec - base);
}
