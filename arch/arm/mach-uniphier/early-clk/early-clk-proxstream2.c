/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>
#include <mach/init.h>
#include <mach/sc-regs.h>

int proxstream2_early_clk_init(const struct uniphier_board_data *bd)
{
	u32 tmp;

	/* deassert reset */
	if (spl_boot_device() != BOOT_DEVICE_NAND) {
		tmp = readl(SC_RSTCTRL);
		tmp &= ~SC_RSTCTRL_NRST_NAND;
		writel(tmp, SC_RSTCTRL);
	};

	tmp = readl(SC_RSTCTRL4);
	tmp |= SC_RSTCTRL4_NRST_UMCSB | SC_RSTCTRL4_NRST_UMCA2 |
	       SC_RSTCTRL4_NRST_UMCA1 | SC_RSTCTRL4_NRST_UMCA0 |
	       SC_RSTCTRL4_NRST_UMC32 | SC_RSTCTRL4_NRST_UMC31 |
	       SC_RSTCTRL4_NRST_UMC30;
	writel(tmp, SC_RSTCTRL4);
	readl(SC_RSTCTRL4); /* dummy read */

	/* privide clocks */
	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_SBC | SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);

	tmp = readl(SC_CLKCTRL4);
	tmp |= SC_CLKCTRL4_CEN_UMCSB | SC_CLKCTRL4_CEN_UMC2 |
	       SC_CLKCTRL4_CEN_UMC1 | SC_CLKCTRL4_CEN_UMC0;
	writel(tmp, SC_CLKCTRL4);
	readl(SC_CLKCTRL4); /* dummy read */

	return 0;
}
