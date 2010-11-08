/*
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
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

/* delay execution before timers are initialized */
static inline void early_udelay(uint32_t usecs)
{
	/* loop takes 4 cycles at 5.0ns (fastest case, running at 200MHz) */
	register uint32_t loops = usecs * (1000 / 20);

	__asm__ volatile ("1:\n"
			"subs %0, %1, #1\n"
			"bne 1b":"=r" (loops):"0" (loops));
}
