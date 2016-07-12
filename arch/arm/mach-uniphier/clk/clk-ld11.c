/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

void uniphier_ld11_clk_init(void)
{
	if (readl(SG_PINMON0) & BIT(27)) {
		/* if booted without stand-by MPU */

		writel(1, SG_ETPHYPSHUT);
		writel(1, SG_ETPHYCNT);

		udelay(1); /* wait for regulator level 1.1V -> 2.5V */

		writel(3, SG_ETPHYCNT);
		writel(3, SG_ETPHYPSHUT);
		writel(7, SG_ETPHYCNT);
	}
}
