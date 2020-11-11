// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/bitops.h>

/* Register offsets */
#define L2_CACHE_CONFIG	0x000
#define L2_CACHE_ENABLE	0x008

#define MASK_NUM_WAYS	GENMASK(15, 8)
#define NUM_WAYS_SHIFT	8

DECLARE_GLOBAL_DATA_PTR;

int cache_enable_ways(void)
{
	const void *blob = gd->fdt_blob;
	int node;
	fdt_addr_t base;
	u32 config;
	u32 ways;

	volatile u32 *enable;

	node = fdt_node_offset_by_compatible(blob, -1,
					     "sifive,fu540-c000-ccache");

	if (node < 0)
		return node;

	base = fdtdec_get_addr_size_auto_parent(blob, 0, node, "reg", 0,
						NULL, false);
	if (base == FDT_ADDR_T_NONE)
		return FDT_ADDR_T_NONE;

	config = readl((volatile u32 *)base + L2_CACHE_CONFIG);
	ways = (config & MASK_NUM_WAYS) >> NUM_WAYS_SHIFT;

	enable = (volatile u32 *)(base + L2_CACHE_ENABLE);

	/* memory barrier */
	mb();
	(*enable) = ways - 1;
	/* memory barrier */
	mb();
	return 0;
}
