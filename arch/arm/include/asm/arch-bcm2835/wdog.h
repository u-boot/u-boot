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

#define BCM2835_WDOG_PHYSADDR			0x20100000

struct bcm2835_wdog_regs {
	u32 unknown0[7];
	u32 rstc;
	u32 unknown1;
	u32 wdog;
};

#define BCM2835_WDOG_PASSWORD			0x5a000000

#define BCM2835_WDOG_RSTC_WRCFG_MASK		0x00000030
#define BCM2835_WDOG_RSTC_WRCFG_FULL_RESET	0x00000020

#define BCM2835_WDOG_WDOG_TIMEOUT_MASK		0x0000ffff

#endif
