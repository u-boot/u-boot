/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
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
#include <asm/processor.h>

/*
 * get_clocks() fills in gd->cpu_clock and gd->bus_clk
 */
int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->cpu_clk = CFG_CLK;
#ifdef CONFIG_M5249
	gd->bus_clk = gd->cpu_clk / 2;
#else
	gd->bus_clk = gd->cpu_clk;
#endif
	return (0);
}
