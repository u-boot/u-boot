/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

void ph1_ld4_clk_init(void)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL);
#ifdef CONFIG_UNIPHIER_ETH
	tmp |= SC_RSTCTRL_NRST_ETHER;
#endif
#ifdef CONFIG_USB_EHCI
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
#ifdef CONFIG_USB_EHCI
	tmp |= SC_CLKCTRL_CEN_MIO | SC_CLKCTRL_CEN_STDMAC;
#endif
#ifdef CONFIG_NAND_DENALI
	tmp |= SC_CLKCTRL_CEN_NAND;
#endif
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */
}
