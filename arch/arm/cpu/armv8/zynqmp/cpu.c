/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

#define ZYNQ_SILICON_VER_MASK	0xF000
#define ZYNQ_SILICON_VER_SHIFT	12

DECLARE_GLOBAL_DATA_PTR;

unsigned int zynqmp_get_silicon_version(void)
{
	gd->cpu_clk = get_tbclk();

	switch (gd->cpu_clk) {
	case 0 ... 1000000:
		return ZYNQMP_CSU_VERSION_VELOCE;
	case 50000000:
		return ZYNQMP_CSU_VERSION_QEMU;
	}

	return ZYNQMP_CSU_VERSION_EP108;
}

#ifndef CONFIG_SYS_DCACHE_OFF
#include <asm/armv8/mmu.h>

#define SECTION_SHIFT_L1	30UL
#define SECTION_SHIFT_L2	21UL
#define BLOCK_SIZE_L0		0x8000000000UL
#define BLOCK_SIZE_L1		(1 << SECTION_SHIFT_L1)
#define BLOCK_SIZE_L2		(1 << SECTION_SHIFT_L2)

#define TCR_TG1_4K		(1 << 31)
#define TCR_EPD1_DISABLE	(1 << 23)
#define ZYNQMO_VA_BITS		40
#define ZYNQMP_TCR		TCR_TG1_4K | \
				TCR_EPD1_DISABLE | \
				TCR_SHARED_OUTER | \
				TCR_SHARED_INNER | \
				TCR_IRGN_WBWA | \
				TCR_ORGN_WBWA | \
				TCR_T0SZ(ZYNQMO_VA_BITS)

#define MEMORY_ATTR	PMD_SECT_AF | PMD_SECT_INNER_SHARE |	\
			PMD_ATTRINDX(MT_NORMAL) |	\
			PMD_TYPE_SECT
#define DEVICE_ATTR	PMD_SECT_AF | PMD_SECT_PXN |	\
			PMD_SECT_UXN | PMD_ATTRINDX(MT_DEVICE_NGNRNE) |	\
			PMD_TYPE_SECT

/* 4K size is required to place 512 entries in each level */
#define TLB_TABLE_SIZE	0x1000

struct attr_tbl {
	u32 num;
	u64 attr;
};

static struct attr_tbl attr_tbll1t0[4] = { {16, 0x0},
					   {8, DEVICE_ATTR},
					   {32, MEMORY_ATTR},
					   {456, DEVICE_ATTR}
					 };
static struct attr_tbl attr_tbll2t3[4] = { {0x180, DEVICE_ATTR},
					   {0x40, 0x0},
					   {0x3F, DEVICE_ATTR},
					   {0x1, MEMORY_ATTR}
					 };

/*
 * This mmu table looks as below
 * Level 0 table contains two entries to 512GB sizes. One is Level1 Table 0
 * and other Level1 Table1.
 * Level1 Table0 contains entries for each 1GB from 0 to 511GB.
 * Level1 Table1 contains entries for each 1GB from 512GB to 1TB.
 * Level2 Table0, Level2 Table1, Level2 Table2 and Level2 Table3 contains
 * entries for each 2MB starting from 0GB, 1GB, 2GB and 3GB respectively.
 */
static void zynqmp_mmu_setup(void)
{
	int el;
	u32 index_attr;
	u64 i, section_l1t0, section_l1t1;
	u64 section_l2t0, section_l2t1, section_l2t2, section_l2t3;
	u64 *level0_table = (u64 *)gd->arch.tlb_addr;
	u64 *level1_table_0 = (u64 *)(gd->arch.tlb_addr + TLB_TABLE_SIZE);
	u64 *level1_table_1 = (u64 *)(gd->arch.tlb_addr + (2 * TLB_TABLE_SIZE));
	u64 *level2_table_0 = (u64 *)(gd->arch.tlb_addr + (3 * TLB_TABLE_SIZE));
	u64 *level2_table_1 = (u64 *)(gd->arch.tlb_addr + (4 * TLB_TABLE_SIZE));
	u64 *level2_table_2 = (u64 *)(gd->arch.tlb_addr + (5 * TLB_TABLE_SIZE));
	u64 *level2_table_3 = (u64 *)(gd->arch.tlb_addr + (6 * TLB_TABLE_SIZE));

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

	index_attr = 0;
	for (i = 0; i < 512; i++) {
		level1_table_0[i] = section_l1t0;
		level1_table_0[i] |= attr_tbll1t0[index_attr].attr;
		attr_tbll1t0[index_attr].num--;
		if (attr_tbll1t0[index_attr].num == 0)
			index_attr++;
		level1_table_1[i] = section_l1t1;
		level1_table_1[i] |= DEVICE_ATTR;
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

	index_attr = 0;

	for (i = 0; i < 512; i++) {
		level2_table_0[i] = section_l2t0 | MEMORY_ATTR;
		level2_table_1[i] = section_l2t1 | MEMORY_ATTR;
		level2_table_2[i] = section_l2t2 | DEVICE_ATTR;
		level2_table_3[i] = section_l2t3 |
				    attr_tbll2t3[index_attr].attr;
		attr_tbll2t3[index_attr].num--;
		if (attr_tbll2t3[index_attr].num == 0)
			index_attr++;
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
			  ZYNQMP_TCR, MEMORY_ATTRIBUTES);

	set_sctlr(get_sctlr() | CR_M);
}

int arch_cpu_init(void)
{
	icache_enable();
	__asm_invalidate_dcache_all();
	__asm_invalidate_tlb_all();
	return 0;
}

/*
 * This function is called from lib/board.c.
 * It recreates MMU table in main memory. MMU and d-cache are enabled earlier.
 * There is no need to disable d-cache for this operation.
 */
void enable_caches(void)
{
	/* The data cache is not active unless the mmu is enabled */
	if (!(get_sctlr() & CR_M)) {
		invalidate_dcache_all();
		__asm_invalidate_tlb_all();
		zynqmp_mmu_setup();
	}
	puts("Enabling Caches...\n");

	set_sctlr(get_sctlr() | CR_C);
}
#endif
