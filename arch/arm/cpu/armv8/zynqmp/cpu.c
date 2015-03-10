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

#define MEMORY_ATTR	PMD_SECT_AF | PMD_SECT_S |	\
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

#if defined(CONFIG_MP)
#define LOCK		0
#define SPLIT		1

#define HALT		0
#define RELEASE		1

#define ZYNQMP_BOOTADDR_HIGH_MASK		0xFFFFFFFF
#define ZYNQMP_R5_HIVEC_ADDR			0xFFFF0000
#define ZYNQMP_R5_LOVEC_ADDR			0x0
#define ZYNQMP_RPU_CFG_CPU_HALT_MASK		0x01
#define ZYNQMP_RPU_CFG_HIVEC_MASK		0x04
#define ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK	0x08
#define ZYNQMP_RPU_GLBL_CTRL_TCM_COMB_MASK	0x40
#define ZYNQMP_RPU_GLBL_CTRL_SLCLAMP_MASK	0x10

#define ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK	0x04
#define ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK	0x01
#define ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK	0x02
#define ZYNQMP_CRLAPB_CPU_R5_CTRL_CLKACT_MASK	0x1000000

#define ZYNQMP_TCM_START_ADDRESS		0xFFE00000
#define ZYNQMP_TCM_BOTH_SIZE			0x40000

#define ZYNQMP_CORE_APU0	0
#define ZYNQMP_CORE_APU3	3

#define ZYNQMP_MAX_CORES	6

int is_core_valid(unsigned int core)
{
	if (core < ZYNQMP_MAX_CORES)
		return 1;

	return 0;
}

int cpu_reset(int nr)
{
	puts("Feature is not implemented.\n");
	return 0;
}

static void set_r5_halt_mode(u8 halt, u8 mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu0_cfg);
	if (halt == HALT)
		tmp &= ~ZYNQMP_RPU_CFG_CPU_HALT_MASK;
	else
		tmp |= ZYNQMP_RPU_CFG_CPU_HALT_MASK;
	writel(tmp, &rpu_base->rpu0_cfg);

	if (mode == LOCK) {
		tmp = readl(&rpu_base->rpu1_cfg);
		if (halt == HALT)
			tmp &= ~ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= ZYNQMP_RPU_CFG_CPU_HALT_MASK;
		writel(tmp, &rpu_base->rpu1_cfg);
	}
}

static void set_r5_tcm_mode(u8 mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu_glbl_ctrl);
	if (mode == LOCK) {
		tmp &= ~ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
		tmp |= ZYNQMP_RPU_GLBL_CTRL_TCM_COMB_MASK |
		       ZYNQMP_RPU_GLBL_CTRL_SLCLAMP_MASK;
	} else {
		tmp |= ZYNQMP_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
		tmp &= ~(ZYNQMP_RPU_GLBL_CTRL_TCM_COMB_MASK |
		       ZYNQMP_RPU_GLBL_CTRL_SLCLAMP_MASK);
	}

	writel(tmp, &rpu_base->rpu_glbl_ctrl);
}

static void set_r5_reset(u8 mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_lpd_top);
	tmp |= (ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
	       ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK);

	if (mode == LOCK)
		tmp |= ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK;

	writel(tmp, &crlapb_base->rst_lpd_top);
}

static void release_r5_reset(u8 mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_lpd_top);
	tmp &= ~(ZYNQMP_CRLAPB_RST_LPD_AMBA_RST_MASK |
	       ZYNQMP_CRLAPB_RST_LPD_R50_RST_MASK);

	if (mode == LOCK)
		tmp &= ~ZYNQMP_CRLAPB_RST_LPD_R51_RST_MASK;

	writel(tmp, &crlapb_base->rst_lpd_top);
}

static void enable_clock_r5(void)
{
	u32 tmp;

	tmp = readl(&crlapb_base->cpu_r5_ctrl);
	tmp |= ZYNQMP_CRLAPB_CPU_R5_CTRL_CLKACT_MASK;
	writel(tmp, &crlapb_base->cpu_r5_ctrl);

	/* Give some delay for clock
	 * to propogate */
	udelay(0x500);
}

int cpu_disable(int nr)
{
	if (nr >= ZYNQMP_CORE_APU0 && nr <= ZYNQMP_CORE_APU3) {
		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val |= 1 << nr;
		writel(val, &crfapb_base->rst_fpd_apu);
	} else {
		set_r5_reset(LOCK);
	}

	return 0;
}

int cpu_status(int nr)
{
	if (nr >= ZYNQMP_CORE_APU0 && nr <= ZYNQMP_CORE_APU3) {
		u32 addr_low = readl(((u8 *)&apu_base->rvbar_addr0_l) + nr * 8);
		u32 addr_high = readl(((u8 *)&apu_base->rvbar_addr0_h) +
				      nr * 8);
		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val &= 1 << nr;
		printf("APU CPU%d %s - starting address HI: %x, LOW: %x\n",
		       nr, val ? "OFF" : "ON" , addr_high, addr_low);
	} else {
		u32 val = readl(&crlapb_base->rst_lpd_top);
		val &= 1 << (nr - 4);
		printf("RPU CPU%d %s\n", nr - 4, val ? "OFF" : "ON");
	}

	return 0;
}

static void set_r5_start(u8 high)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu0_cfg);
	if (high)
		tmp |= ZYNQMP_RPU_CFG_HIVEC_MASK;
	else
		tmp &= ~ZYNQMP_RPU_CFG_HIVEC_MASK;
	writel(tmp, &rpu_base->rpu0_cfg);

	tmp = readl(&rpu_base->rpu1_cfg);
	if (high)
		tmp |= ZYNQMP_RPU_CFG_HIVEC_MASK;
	else
		tmp &= ~ZYNQMP_RPU_CFG_HIVEC_MASK;
	writel(tmp, &rpu_base->rpu1_cfg);
}

int cpu_release(int nr, int argc, char * const argv[])
{
	if (nr >= ZYNQMP_CORE_APU0 && nr <= ZYNQMP_CORE_APU3) {
		u64 boot_addr = simple_strtoull(argv[0], NULL, 16);
		/* HIGH */
		writel((u32)(boot_addr >> 32),
		       ((u8 *)&apu_base->rvbar_addr0_h) + nr * 8);
		/* LOW */
		writel((u32)(boot_addr & ZYNQMP_BOOTADDR_HIGH_MASK),
		       ((u8 *)&apu_base->rvbar_addr0_l) + nr * 8);

		u32 val = readl(&crfapb_base->rst_fpd_apu);
		val &= ~(1 << nr);
		writel(val, &crfapb_base->rst_fpd_apu);
	} else {
		if (argc != 2) {
			printf("Invalid number of arguments to release.\n");
			printf("<addr> <mode>-Start addr lockstep or split\n");
			return 1;
		}

		u32 boot_addr = simple_strtoul(argv[0], NULL, 16);
		if (!(boot_addr == ZYNQMP_R5_LOVEC_ADDR ||
		      boot_addr == ZYNQMP_R5_HIVEC_ADDR)) {
			printf("Invalid starting address 0x%x\n", boot_addr);
			printf("0 or 0xffff0000 are permitted\n");
			return 1;
		}

		if (!strncmp(argv[1], "lockstep", 8)) {
			printf("R5 lockstep mode\n");
			set_r5_tcm_mode(LOCK);
			set_r5_halt_mode(HALT, LOCK);

			if (boot_addr == 0)
				set_r5_start(0);
			else
				set_r5_start(1);

			enable_clock_r5();
			release_r5_reset(LOCK);
			set_r5_halt_mode(RELEASE, LOCK);
		} else if (!strncmp(argv[1], "split", 5)) {
			printf("R5 split mode\n");
			set_r5_tcm_mode(SPLIT);
			set_r5_halt_mode(HALT, SPLIT);
			enable_clock_r5();
			release_r5_reset(SPLIT);
			set_r5_halt_mode(RELEASE, SPLIT);
		} else {
			printf("Unsupported mode\n");
			return 1;
		}
	}

	return 0;
}
#endif /* CONFIG_MP */
