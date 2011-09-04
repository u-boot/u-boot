/*
 * Copyright (c) 2011 The Chromium OS Authors.
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

/* Tegra2 pin multiplexing functions */

#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/pinmux.h>
#include <common.h>


void pinmux_set_tristate(enum pmux_pin pin, int enable)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;
	u32 *tri = &pmt->pmt_tri[TRISTATE_REG(pin)];
	u32 reg;

	reg = readl(tri);
	if (enable)
		reg |= TRISTATE_MASK(pin);
	else
		reg &= ~TRISTATE_MASK(pin);
	writel(reg, tri);
}

void pinmux_tristate_enable(enum pmux_pin pin)
{
	pinmux_set_tristate(pin, 1);
}

void pinmux_tristate_disable(enum pmux_pin pin)
{
	pinmux_set_tristate(pin, 0);
}
