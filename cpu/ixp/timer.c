/* vi: set ts=8 sw=8 noet: */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
#include <asm/arch/ixp425.h>

ulong get_timer (ulong base)
{
       return get_timer_masked () - base;
}

void ixp425_udelay(unsigned long usec)
{
	/*
	 * This function has a max usec, but since it is called from udelay
	 * we should not have to worry... be happy
	 */
	unsigned long usecs = CFG_HZ/1000000L & ~IXP425_OST_RELOAD_MASK;

	*IXP425_OSST = IXP425_OSST_TIMER_1_PEND;
	usecs |= IXP425_OST_ONE_SHOT | IXP425_OST_ENABLE;
	*IXP425_OSRT1 = usecs;
	while (!(*IXP425_OSST & IXP425_OSST_TIMER_1_PEND));
}

void udelay (unsigned long usec)
{
	while (usec--) ixp425_udelay(1);
}

static ulong reload_constant = 0xfffffff0;

void reset_timer_masked (void)
{
	ulong reload = reload_constant | IXP425_OST_ONE_SHOT | IXP425_OST_ENABLE;

	*IXP425_OSST = IXP425_OSST_TIMER_1_PEND;
	*IXP425_OSRT1 = reload;
}

ulong get_timer_masked (void)
{
	/*
	 * Note that it is possible for this to wrap!
	 * In this case we return max.
	 */
	ulong current = *IXP425_OST1;
	if (*IXP425_OSST & IXP425_OSST_TIMER_1_PEND)
	{
		return reload_constant;
	}
	return (reload_constant - current);
}
