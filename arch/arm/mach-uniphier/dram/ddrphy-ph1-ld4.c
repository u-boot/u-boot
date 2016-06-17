/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/io.h>

#include "ddrphy-regs.h"

enum dram_freq {
	DRAM_FREQ_1333M,
	DRAM_FREQ_1600M,
	DRAM_FREQ_NR,
};

static u32 ddrphy_ptr0[DRAM_FREQ_NR] = {0x0a806844, 0x0c807d04};
static u32 ddrphy_ptr1[DRAM_FREQ_NR] = {0x208e0124, 0x2710015E};
static u32 ddrphy_ptr3[DRAM_FREQ_NR] = {0x0f051616, 0x12061A80};
static u32 ddrphy_ptr4[DRAM_FREQ_NR] = {0x06ae08d6, 0x08027100};
static u32 ddrphy_dtpr0[DRAM_FREQ_NR] = {0x85589955, 0x999cbb66};
static u32 ddrphy_dtpr1[DRAM_FREQ_NR] = {0x1a8363c0, 0x1a878400};
static u32 ddrphy_dtpr2[DRAM_FREQ_NR] = {0x5002c200, 0xa00214f8};
static u32 ddrphy_mr0[DRAM_FREQ_NR] = {0x00000b51, 0x00000d71};
static u32 ddrphy_mr2[DRAM_FREQ_NR] = {0x00000290, 0x00000298};

int ph1_ld4_ddrphy_init(struct ddrphy __iomem *phy, int freq, bool ddr3plus)
{
	enum dram_freq freq_e;
	u32 tmp;

	switch (freq) {
	case 1333:
		freq_e = DRAM_FREQ_1333M;
		break;
	case 1600:
		freq_e = DRAM_FREQ_1600M;
		break;
	default:
		printf("unsupported DRAM frequency %d MHz\n", freq);
		return -EINVAL;
	}

	writel(0x0300c473, &phy->pgcr[1]);
	writel(ddrphy_ptr0[freq_e], &phy->ptr[0]);
	writel(ddrphy_ptr1[freq_e], &phy->ptr[1]);
	writel(0x00083DEF, &phy->ptr[2]);
	writel(ddrphy_ptr3[freq_e], &phy->ptr[3]);
	writel(ddrphy_ptr4[freq_e], &phy->ptr[4]);
	writel(0xF004001A, &phy->dsgcr);

	/* change the value of the on-die pull-up/pull-down registors */
	tmp = readl(&phy->dxccr);
	tmp &= ~0x0ee0;
	tmp |= DXCCR_DQSNRES_688_OHM | DXCCR_DQSRES_688_OHM;
	writel(tmp, &phy->dxccr);

	writel(0x0000040B, &phy->dcr);
	writel(ddrphy_dtpr0[freq_e], &phy->dtpr[0]);
	writel(ddrphy_dtpr1[freq_e], &phy->dtpr[1]);
	writel(ddrphy_dtpr2[freq_e], &phy->dtpr[2]);
	writel(ddrphy_mr0[freq_e], &phy->mr0);
	writel(0x00000006, &phy->mr1);
	writel(ddrphy_mr2[freq_e], &phy->mr2);
	writel(ddr3plus ? 0x00000800 : 0x00000000, &phy->mr3);

	while (!(readl(&phy->pgsr[0]) & PGSR0_IDONE))
		;

	writel(0x0300C473, &phy->pgcr[1]);
	writel(0x0000005D, &phy->zq[0].cr[1]);

	return 0;
}
