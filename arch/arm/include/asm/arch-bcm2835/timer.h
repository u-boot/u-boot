/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _BCM2835_TIMER_H
#define _BCM2835_TIMER_H

#define BCM2835_TIMER_PHYSADDR	0x20003000

struct bcm2835_timer_regs {
	u32 cs;
	u32 clo;
	u32 chi;
	u32 c0;
	u32 c1;
	u32 c2;
	u32 c3;
};

#define BCM2835_TIMER_CS_M3	(1 << 3)
#define BCM2835_TIMER_CS_M2	(1 << 2)
#define BCM2835_TIMER_CS_M1	(1 << 1)
#define BCM2835_TIMER_CS_M0	(1 << 0)

#endif
