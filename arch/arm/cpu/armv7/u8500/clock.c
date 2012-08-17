/*
 * (C) Copyright 2009 ST-Ericsson
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
#include <asm/io.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

struct clkrst {
	unsigned int pcken;
	unsigned int pckdis;
	unsigned int kcken;
	unsigned int kckdis;
};

static unsigned int clkrst_base[] = {
	U8500_CLKRST1_BASE,
	U8500_CLKRST2_BASE,
	U8500_CLKRST3_BASE,
	0,
	U8500_CLKRST5_BASE,
	U8500_CLKRST6_BASE,
	U8500_CLKRST7_BASE,	/* ED only */
};

/* Turn on peripheral clock at PRCC level */
void u8500_clock_enable(int periph, int cluster, int kern)
{
	struct clkrst *clkrst = (struct clkrst *) clkrst_base[periph - 1];

	if (kern != -1)
		writel(1 << kern, &clkrst->kcken);

	if (cluster != -1)
		writel(1 << cluster, &clkrst->pcken);
}
