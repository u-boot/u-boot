/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sc-regs.h>

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
	tmp |= SC_CLKCTRL_CLK_ETHER | SC_CLKCTRL_CLK_MIO | SC_CLKCTRL_CLK_UMC
	     | SC_CLKCTRL_CLK_NAND | SC_CLKCTRL_CLK_SBC | SC_CLKCTRL_CLK_PERI;
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */
}
