/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>
#include <mach/sg-regs.h>

void sg_init(void)
{
	u32 tmp;

	/* Input ports must be enabled before deasserting reset of cores */
	tmp = readl(SG_IECTRL);
	tmp |= 1 << 6;
	writel(tmp, SG_IECTRL);
}
