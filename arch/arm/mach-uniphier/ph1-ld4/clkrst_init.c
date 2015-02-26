/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <mach/sc-regs.h>

void clkrst_init(void)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL);
	tmp |= SC_RSTCTRL_NRST_ETHER | SC_RSTCTRL_NRST_UMC1
		| SC_RSTCTRL_NRST_UMC0 | SC_RSTCTRL_NRST_NAND;
	writel(tmp, SC_RSTCTRL);
	readl(SC_RSTCTRL); /* dummy read */

	/* privide clocks */
	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_ETHER | SC_CLKCTRL_CEN_MIO | SC_CLKCTRL_CEN_UMC
	     | SC_CLKCTRL_CEN_NAND | SC_CLKCTRL_CEN_SBC | SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */
}
