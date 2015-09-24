/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/io.h>
#include <mach/init.h>
#include <mach/sc-regs.h>

int ph1_ld4_early_clk_init(const struct uniphier_board_data *bd)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL);

	tmp |= SC_RSTCTRL_NRST_UMC1 | SC_RSTCTRL_NRST_UMC0;
	if (spl_boot_device() != BOOT_DEVICE_NAND)
		tmp &= ~SC_RSTCTRL_NRST_NAND;
	writel(tmp, SC_RSTCTRL);
	readl(SC_RSTCTRL); /* dummy read */

	/* privide clocks */
	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_UMC | SC_CLKCTRL_CEN_SBC | SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);
	readl(SC_CLKCTRL); /* dummy read */

	return 0;
}
