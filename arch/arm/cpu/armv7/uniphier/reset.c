/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sc-regs.h>

void reset_cpu(unsigned long ignored)
{
	u32 tmp;

	writel(5, SC_IRQTIMSET); /* default value */

	tmp  = readl(SC_SLFRSTSEL);
	tmp &= ~0x3; /* mask [1:0] */
	tmp |= 0x0;  /* XRST reboot */
	writel(tmp, SC_SLFRSTSEL);

	tmp = readl(SC_SLFRSTCTL);
	tmp |= 0x1;
	writel(tmp, SC_SLFRSTCTL);
}
