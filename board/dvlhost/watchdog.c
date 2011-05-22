/*
 * (C) Copyright 2009
 * Michael Schwingen, michael@schwingen.org
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include "dvlhost_hw.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#include <asm/arch/ixp425.h>

void hw_watchdog_reset(void)
{
	unsigned int x;
	x = readl(IXP425_GPIO_GPOUTR);
	x ^= (1 << (CONFIG_SYS_GPIO_WDGTRIGGER));
	writel(x, IXP425_GPIO_GPOUTR);
}

#endif /* CONFIG_HW_WATCHDOG */
