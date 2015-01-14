/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ddrphy-regs.h>

void ddrphy_prepare_training(struct ddrphy __iomem *phy, int rank)
{
	int dx;
	u32 __iomem tmp, *p;

	for (dx = 0; dx < NR_DATX8_PER_DDRPHY; dx++) {
		p = &phy->dx[dx].gcr;

		tmp = readl(p);
		/* Specify the rank that should be write leveled */
		tmp &= ~DXGCR_WLRKEN_MASK;
		tmp |= (1 << (DXGCR_WLRKEN_SHIFT + rank)) & DXGCR_WLRKEN_MASK;
		writel(tmp, p);
	}

	p = &phy->dtcr;

	tmp = readl(p);
	/* Specify the rank used during data bit deskew and eye centering */
	tmp &= ~DTCR_DTRANK_MASK;
	tmp |= (rank << DTCR_DTRANK_SHIFT) & DTCR_DTRANK_MASK;
	/* Use Multi-Purpose Register for DQS gate training */
	tmp |= DTCR_DTMPR;
	/* Specify the rank enabled for data-training */
	tmp &= ~DTCR_RNKEN_MASK;
	tmp |= (1 << (DTCR_RNKEN_SHIFT + rank)) & DTCR_RNKEN_MASK;
	writel(tmp, p);
}

struct ddrphy_init_sequence {
	char *description;
	u32 init_flag;
	u32 done_flag;
	u32 err_flag;
};

static struct ddrphy_init_sequence init_sequence[] = {
	{
		"DRAM Initialization",
		PIR_DRAMRST | PIR_DRAMINIT,
		PGSR0_DIDONE,
		PGSR0_DIERR
	},
	{
		"Write Leveling",
		PIR_WL,
		PGSR0_WLDONE,
		PGSR0_WLERR
	},
	{
		"Read DQS Gate Training",
		PIR_QSGATE,
		PGSR0_QSGDONE,
		PGSR0_QSGERR
	},
	{
		"Write Leveling Adjustment",
		PIR_WLADJ,
		PGSR0_WLADONE,
		PGSR0_WLAERR
	},
	{
		"Read Bit Deskew",
		PIR_RDDSKW,
		PGSR0_RDDONE,
		PGSR0_RDERR
	},
	{
		"Write Bit Deskew",
		PIR_WRDSKW,
		PGSR0_WDDONE,
		PGSR0_WDERR
	},
	{
		"Read Eye Training",
		PIR_RDEYE,
		PGSR0_REDONE,
		PGSR0_REERR
	},
	{
		"Write Eye Training",
		PIR_WREYE,
		PGSR0_WEDONE,
		PGSR0_WEERR
	}
};

int ddrphy_training(struct ddrphy __iomem *phy)
{
	int i;
	u32 pgsr0;
	u32 init_flag = PIR_INIT;
	u32 done_flag = PGSR0_IDONE;
	int timeout = 50000; /* 50 msec is long enough */
#ifdef DISPLAY_ELAPSED_TIME
	ulong start = get_timer(0);
#endif

	for (i = 0; i < ARRAY_SIZE(init_sequence); i++) {
		init_flag |= init_sequence[i].init_flag;
		done_flag |= init_sequence[i].done_flag;
	}

	writel(init_flag, &phy->pir);

	do {
		if (--timeout < 0) {
#ifndef CONFIG_SPL_BUILD
			printf("%s: error: timeout during DDR training\n",
								__func__);
#endif
			return -1;
		}
		udelay(1);
		pgsr0 = readl(&phy->pgsr[0]);
	} while ((pgsr0 & done_flag) != done_flag);

	for (i = 0; i < ARRAY_SIZE(init_sequence); i++) {
		if (pgsr0 & init_sequence[i].err_flag) {
#ifndef CONFIG_SPL_BUILD
			printf("%s: error: %s failed\n", __func__,
						init_sequence[i].description);
#endif
			return -1;
		}
	}

#ifdef DISPLAY_ELAPSED_TIME
	printf("%s: info: elapsed time %ld msec\n", get_timer(start));
#endif

	return 0;
}
