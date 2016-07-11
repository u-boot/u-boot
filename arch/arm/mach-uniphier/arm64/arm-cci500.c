/*
 * Initialization of ARM Corelink CCI-500 Cache Coherency Interconnect
 *
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mapmem.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#define CCI500_BASE			0x5FD00000
#define CCI500_SLAVE_OFFSET		0x1000

#define CCI500_SNOOP_CTRL
#define   CCI500_SNOOP_CTRL_EN_DVM	BIT(1)
#define   CCI500_SNOOP_CTRL_EN_SNOOP	BIT(0)

void cci500_init(unsigned int nr_slaves)
{
	unsigned long slave_base = CCI500_BASE + CCI500_SLAVE_OFFSET;
	int i;

	for (i = 0; i < nr_slaves; i++) {
		void __iomem *base;
		u32 tmp;

		base = map_sysmem(slave_base, SZ_4K);

		tmp = readl(base);
		tmp |= CCI500_SNOOP_CTRL_EN_DVM | CCI500_SNOOP_CTRL_EN_SNOOP;
		writel(tmp, base);

		unmap_sysmem(base);

		slave_base += CCI500_SLAVE_OFFSET;
	}
}
