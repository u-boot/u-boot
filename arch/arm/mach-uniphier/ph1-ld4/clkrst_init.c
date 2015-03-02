/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <mach/sc-regs.h>

void clkrst_init(void)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL);
#ifdef CONFIG_UNIPHIER_ETH
	tmp |= SC_RSTCTRL_NRST_ETHER;
#endif
#ifdef CONFIG_USB_EHCI_UNIPHIER
	tmp |= SC_RSTCTRL_NRST_STDMAC;
#endif
#ifdef CONFIG_NAND_DENALI
	tmp |= SC_RSTCTRL_NRST_NAND;
#endif
	writel(tmp, SC_RSTCTRL);
	readl(SC_RSTCTRL); /* dummy read */

	/* privide clocks */
	tmp = readl(SC_CLKCTRL);
#ifdef CONFIG_UNIPHIER_ETH
	tmp |= SC_CLKCTRL_CEN_ETHER;
#endif
#ifdef CONFIG_USB_EHCI_UNIPHIER
	tmp |= SC_CLKCTRL_CEN_MIO | SC_CLKCTRL_CEN_STDMAC;
#endif
#ifdef CONFIG_NAND_DENALI
	tmp |= SC_CLKCTRL_CEN_NAND;
#endif
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */
}
