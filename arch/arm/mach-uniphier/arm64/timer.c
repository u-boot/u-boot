/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#define CNT_CONTROL_BASE	0x60E00000

#define CNTCR			0x000
#define   CNTCR_EN			BIT(0)

/* setup ARMv8 Generic Timer */
int timer_init(void)
{
	void __iomem *base;
	u32 tmp;

	base = ioremap(CNT_CONTROL_BASE, SZ_4K);

	/*
	 * Note:
	 * In a system that implements both Secure and Non-secure states,
	 * this register is only writable in Secure state.
	 */
	tmp = readl(base + CNTCR);
	tmp |= CNTCR_EN;
	writel(tmp, base + CNTCR);

	iounmap(base);

	return 0;
}
