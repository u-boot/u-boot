/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 * (This file derived from arch/arm/cpu/armv8/zynqmp/cpu.c)
 *
 * Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

#define SECTION_SHIFT_L1	30UL
#define SECTION_SHIFT_L2	21UL
#define BLOCK_SIZE_L0		0x8000000000UL
#define BLOCK_SIZE_L1		(1 << SECTION_SHIFT_L1)
#define BLOCK_SIZE_L2		(1 << SECTION_SHIFT_L2)

#define TCR_TG1_4K		(1 << 31)
#define TCR_EPD1_DISABLE	(1 << 23)
#define TEGRA_VA_BITS		40
#define TEGRA_TCR		TCR_TG1_4K | \
				TCR_EPD1_DISABLE | \
				TCR_SHARED_OUTER | \
				TCR_SHARED_INNER | \
				TCR_IRGN_WBWA | \
				TCR_ORGN_WBWA | \
				TCR_T0SZ(TEGRA_VA_BITS)

#define MEMORY_ATTR	PMD_SECT_AF | PMD_SECT_INNER_SHARE |	\
			PMD_ATTRINDX(MT_NORMAL) |	\
			PMD_TYPE_SECT
#define DEVICE_ATTR	PMD_SECT_AF | PMD_SECT_PXN |	\
			PMD_SECT_UXN | PMD_ATTRINDX(MT_DEVICE_NGNRNE) |	\
			PMD_TYPE_SECT

/* 4K size is required to place 512 entries in each level */
#define TLB_TABLE_SIZE	0x1000

/*
 * This mmu table looks as below
 * Level 0 table contains two entries to 512GB sizes. One is Level1 Table 0
 * and other Level1 Table1.
 * Level1 Table0 contains entries for each 1GB from 0 to 511GB.
 * Level1 Table1 contains entries for each 1GB from 512GB to 1TB.
 * Level2 Table0, Level2 Table1, Level2 Table2 and Level2 Table3 contains
 * entries for each 2MB starting from 0GB, 1GB, 2GB and 3GB respectively.
 */
void mmu_setup(void)
{
	int el;
	u64 i, section_l1t0, section_l1t1;
	u64 section_l2t0, section_l2t1, section_l2t2, section_l2t3;
	u64 *level0_table = (u64 *)gd->arch.tlb_addr;
	u64 *level1_table_0 = (u64 *)(gd->arch.tlb_addr + TLB_TABLE_SIZE);
	u64 *level1_table_1 = (u64 *)(gd->arch.tlb_addr + (2 * TLB_TABLE_SIZE));
	u64 *level2_table_0 = (u64 *)(gd->arch.tlb_addr + (3 * TLB_TABLE_SIZE));
	u64 *level2_table_1 = (u64 *)(gd->arch.tlb_addr + (4 * TLB_TABLE_SIZE));
	u64 *level2_table_2 = (u64 *)(gd->arch.tlb_addr + (5 * TLB_TABLE_SIZE));
	u64 *level2_table_3 = (u64 *)(gd->arch.tlb_addr + (6 * TLB_TABLE_SIZE));

	/* Invalidate all table entries */
	memset(level0_table, 0, PGTABLE_SIZE);

	level0_table[0] =
		(u64)level1_table_0 | PMD_TYPE_TABLE;
	level0_table[1] =
		(u64)level1_table_1 | PMD_TYPE_TABLE;

	/*
	 * set level 1 table 0, covering 0 to 512GB
	 * set level 1 table 1, covering 512GB to 1TB
	 */
	section_l1t0 = 0;
	section_l1t1 = BLOCK_SIZE_L0;

	for (i = 0; i < 512; i++) {
		level1_table_0[i] = section_l1t0;
		if (i >= 4)
			level1_table_0[i] |= MEMORY_ATTR;
		level1_table_1[i] = section_l1t1;
		level1_table_1[i] |= MEMORY_ATTR;
		section_l1t0 += BLOCK_SIZE_L1;
		section_l1t1 += BLOCK_SIZE_L1;
	}

	level1_table_0[0] =
		(u64)level2_table_0 | PMD_TYPE_TABLE;
	level1_table_0[1] =
		(u64)level2_table_1 | PMD_TYPE_TABLE;
	level1_table_0[2] =
		(u64)level2_table_2 | PMD_TYPE_TABLE;
	level1_table_0[3] =
		(u64)level2_table_3 | PMD_TYPE_TABLE;

	section_l2t0 = 0;
	section_l2t1 = section_l2t0 + BLOCK_SIZE_L1; /* 1GB */
	section_l2t2 = section_l2t1 + BLOCK_SIZE_L1; /* 2GB */
	section_l2t3 = section_l2t2 + BLOCK_SIZE_L1; /* 3GB */

	for (i = 0; i < 512; i++) {
		level2_table_0[i] = section_l2t0 | DEVICE_ATTR;
		level2_table_1[i] = section_l2t1 | DEVICE_ATTR;
		level2_table_2[i] = section_l2t2 | MEMORY_ATTR;
		level2_table_3[i] = section_l2t3 | MEMORY_ATTR;
		section_l2t0 += BLOCK_SIZE_L2;
		section_l2t1 += BLOCK_SIZE_L2;
		section_l2t2 += BLOCK_SIZE_L2;
		section_l2t3 += BLOCK_SIZE_L2;
	}

	/* flush new MMU table */
	flush_dcache_range(gd->arch.tlb_addr,
			   gd->arch.tlb_addr + gd->arch.tlb_size);

	/* point TTBR to the new table */
	el = current_el();
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr,
			  TEGRA_TCR, MEMORY_ATTRIBUTES);

	set_sctlr(get_sctlr() | CR_M);
}

u64 *arch_get_page_table(void)
{
	return (u64 *)(gd->arch.tlb_addr + (3 * TLB_TABLE_SIZE));
}
