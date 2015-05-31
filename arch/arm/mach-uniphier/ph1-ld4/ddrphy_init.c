/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/types.h>
#include <linux/io.h>
#include <mach/ddrphy-regs.h>

void ddrphy_init(struct ddrphy __iomem *phy, int freq, int size)
{
	u32 tmp;

	writel(0x0300c473, &phy->pgcr[1]);
	if (freq == 1333) {
		writel(0x0a806844, &phy->ptr[0]);
		writel(0x208e0124, &phy->ptr[1]);
	} else {
		writel(0x0c807d04, &phy->ptr[0]);
		writel(0x2710015E, &phy->ptr[1]);
	}
	writel(0x00083DEF, &phy->ptr[2]);
	if (freq == 1333) {
		writel(0x0f051616, &phy->ptr[3]);
		writel(0x06ae08d6, &phy->ptr[4]);
	} else {
		writel(0x12061A80, &phy->ptr[3]);
		writel(0x08027100, &phy->ptr[4]);
	}
	writel(0xF004001A, &phy->dsgcr);

	/* change the value of the on-die pull-up/pull-down registors */
	tmp = readl(&phy->dxccr);
	tmp &= ~0x0ee0;
	tmp |= DXCCR_DQSNRES_688_OHM | DXCCR_DQSRES_688_OHM;
	writel(tmp, &phy->dxccr);

	writel(0x0000040B, &phy->dcr);
	if (freq == 1333) {
		writel(0x85589955, &phy->dtpr[0]);
		if (size == 1)
			writel(0x1a8253c0, &phy->dtpr[1]);
		else
			writel(0x1a8363c0, &phy->dtpr[1]);
		writel(0x5002c200, &phy->dtpr[2]);
		writel(0x00000b51, &phy->mr0);
	} else {
		writel(0x999cbb66, &phy->dtpr[0]);
		if (size == 1)
			writel(0x1a82dbc0, &phy->dtpr[1]);
		else
			writel(0x1a878400, &phy->dtpr[1]);
		writel(0xa00214f8, &phy->dtpr[2]);
		writel(0x00000d71, &phy->mr0);
	}
	writel(0x00000006, &phy->mr1);
	if (freq == 1333)
		writel(0x00000290, &phy->mr2);
	else
		writel(0x00000298, &phy->mr2);

	writel(0x00000800, &phy->mr3);

	while (!(readl(&phy->pgsr[0]) & PGSR0_IDONE))
		;

	writel(0x0300C473, &phy->pgcr[1]);
	writel(0x0000005D, &phy->zq[0].cr[1]);
}
