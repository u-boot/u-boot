/*
 * (C) Copyright 2010
 * Matthias Weisser <weisserm@arcor.de>
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
#include <asm/io.h>
#include <asm/arch/hardware.h>

/*
 * Get the peripheral bus frequency depending on pll pin settings
 */
ulong get_bus_freq(ulong dummy)
{
	struct mb86r0x_crg * crg = (struct mb86r0x_crg *)
					MB86R0x_CRG_BASE;
	uint32_t pllmode;

	pllmode = readl(&crg->crpr) & MB86R0x_CRG_CRPR_PLLMODE;

	if (pllmode == MB86R0x_CRG_CRPR_PLLMODE_X20)
		return 40000000;

	return 41164767;
}
