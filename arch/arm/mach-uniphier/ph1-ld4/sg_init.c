/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <mach/sg-regs.h>

void sg_init(void)
{
	u32 tmp;

	/* Input ports must be enabled before deasserting reset of cores */
	tmp = readl(SG_IECTRL);
	tmp |= 0x1;
	writel(tmp, SG_IECTRL);
}
