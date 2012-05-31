/*
 * (C) Copyright 2010 Linaro
 * Matt Waddel, <matt.waddel@linaro.org>
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
#ifndef _SYSTIMER_H_
#define _SYSTIMER_H_

/* AMBA timer register base address */
#define SYSTIMER_BASE		0x10011000

#define SYSHZ_CLOCK		1000000		/* Timers -> 1Mhz */
#define SYSTIMER_RELOAD		0xFFFFFFFF
#define SYSTIMER_EN		(1 << 7)
#define SYSTIMER_32BIT		(1 << 1)

struct systimer {
	u32 timer0load;		/* 0x00 */
	u32 timer0value;
	u32 timer0control;
	u32 timer0intclr;
	u32 timer0ris;
	u32 timer0mis;
	u32 timer0bgload;
	u32 timer1load;		/* 0x20 */
	u32 timer1value;
	u32 timer1control;
	u32 timer1intclr;
	u32 timer1ris;
	u32 timer1mis;
	u32 timer1bgload;
};
#endif /* _SYSTIMER_H_ */
