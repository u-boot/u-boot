// SPDX-License-Identifier: GPL-2.0+
/*
 * TI clock utilities
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include "clk.h"

static void clk_ti_rmw(u32 val, u32 mask, fdt_addr_t reg)
{
	u32 v;

	v = readl(reg);
	v &= ~mask;
	v |= val;
	writel(v, reg);
}

void clk_ti_latch(fdt_addr_t reg, s8 shift)
{
	u32 latch;

	if (shift < 0)
		return;

	latch = 1 << shift;

	clk_ti_rmw(latch, latch, reg);
	clk_ti_rmw(0, latch, reg);
	readl(reg);		/* OCP barrier */
}
