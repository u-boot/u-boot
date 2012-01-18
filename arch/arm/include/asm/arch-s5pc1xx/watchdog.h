/*
 * Copyright (C) 2011 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
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

#ifndef __ASM_ARM_ARCH_WATCHDOG_H_
#define __ASM_ARM_ARCH_WATCHDOG_H_

#define WTCON_RESET_OFFSET	0
#define WTCON_INTEN_OFFSET	2
#define WTCON_CLKSEL_OFFSET	3
#define WTCON_EN_OFFSET		5
#define WTCON_PRE_OFFSET	8

#define WTCON_CLK_16		0x0
#define WTCON_CLK_32		0x1
#define WTCON_CLK_64		0x2
#define WTCON_CLK_128		0x3

#define WTCON_CLK(x)		((x & 0x3) << WTCON_CLKSEL_OFFSET)
#define WTCON_PRESCALER(x)	((x) << WTCON_PRE_OFFSET)
#define WTCON_EN		(0x1 << WTCON_EN_OFFSET)
#define WTCON_RESET		(0x1 << WTCON_RESET_OFFSET)
#define WTCON_INT		(0x1 << WTCON_INTEN_OFFSET)

#ifndef __ASSEMBLY__
struct s5p_watchdog {
	unsigned int wtcon;
	unsigned int wtdat;
	unsigned int wtcnt;
	unsigned int wtclrint;
};

/* functions */
void wdt_stop(void);
void wdt_start(unsigned int timeout);
#endif	/* __ASSEMBLY__ */

#endif
