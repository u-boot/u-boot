/*
 * PLL setup for Cirrus edb93xx boards
 *
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2006 Dominic Rath <Dominic.Rath@gmx.de>
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
#include "pll_cfg.h"
#include "early_udelay.h"

void pll_cfg(void)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;

	/* setup PLL1 */
	writel(CLKSET1_VAL, &syscon->clkset1);

	/*
	 * flush the pipeline
	 * writing to CLKSET1 causes the EP93xx to enter standby for between
	 * 8 ms to 16 ms, until PLL1 stabilizes
	 */
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");

	/* setup PLL2 */
	writel(CLKSET2_VAL, &syscon->clkset2);

	/*
	 * the user's guide recommends to wait at least 1 ms for PLL2 to
	 * stabilize
	 */
	early_udelay(1000);
}
