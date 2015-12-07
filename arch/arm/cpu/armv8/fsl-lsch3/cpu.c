/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-fsl-lsch3/soc.h>
#include <asm/arch-fsl-lsch3/immap_lsch3.h>
#include <fsl_debug_server.h>
#include <fsl-mc/fsl_mc.h>
#include <asm/arch/fsl_serdes.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif
#include "cpu.h"
#include "mp.h"
#include "speed.h"

DECLARE_GLOBAL_DATA_PTR;

static struct cpu_type cpu_type_list[] = {
#ifdef CONFIG_LS2085A
	CPU_TYPE_ENTRY(LS2085, LS2085, 8),
	CPU_TYPE_ENTRY(LS2080, LS2080, 8),
	CPU_TYPE_ENTRY(LS2045, LS2045, 4),
#endif
};

void cpu_name(char *name)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int i, svr, ver;

	svr = in_le32(&gur->svr);
	ver = SVR_SOC_VER(svr);

	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++)
		if ((cpu_type_list[i].soc_ver & SVR_WO_E) == ver) {
			strcpy(name, cpu_type_list[i].name);

			if (IS_E_PROCESSOR(svr))
				strcat(name, "E");
			break;
		}

	if (i == ARRAY_SIZE(cpu_type_list))
		strcpy(name, "unknown");
}

#ifndef CONFIG_SYS_DCACHE_OFF

#define SECTION_SHIFT_L0		39UL
#define SECTION_SHIFT_L1		30UL
#define SECTION_SHIFT_L2		21UL
#define BLOCK_SIZE_L0			0x8000000000
#define BLOCK_SIZE_L1			0x40000000
#define BLOCK_SIZE_L2			0x200000

#define NUM_OF_ENTRY		512

#define TCR_EL2_PS_40BIT	(2 << 16)
#define LSCH3_VA_BITS		(40)
#define LSCH3_TCR	(TCR_TG0_4K		| \
			TCR_EL2_PS_40BIT	| \
			TCR_SHARED_NON		| \
			TCR_ORGN_NC		| \
			TCR_IRGN_NC		| \
			TCR_T0SZ(LSCH3_VA_BITS))
#define LSCH3_TCR_FINAL	(TCR_TG0_4K		| \
			TCR_EL2_PS_40BIT	| \
			TCR_SHARED_OUTER	| \
			TCR_ORGN_WBWA		| \
			TCR_IRGN_WBWA		| \
			TCR_T0SZ(LSCH3_VA_BITS))

#define CONFIG_SYS_FSL_CCSR_BASE	0x00000000
#define CONFIG_SYS_FSL_CCSR_SIZE	0x10000000
#define CONFIG_SYS_FSL_QSPI_BASE1	0x20000000
#define CONFIG_SYS_FSL_QSPI_SIZE1	0x10000000
#define CONFIG_SYS_FSL_IFC_BASE1	0x30000000
#define CONFIG_SYS_FSL_IFC_SIZE1	0x10000000
#define CONFIG_SYS_FSL_IFC_SIZE1_1	0x400000
#define CONFIG_SYS_FSL_DRAM_BASE1	0x80000000
#define CONFIG_SYS_FSL_DRAM_SIZE1	0x80000000
#define CONFIG_SYS_FSL_QSPI_BASE2	0x400000000
#define CONFIG_SYS_FSL_QSPI_SIZE2	0x100000000
#define CONFIG_SYS_FSL_IFC_BASE2	0x500000000
#define CONFIG_SYS_FSL_IFC_SIZE2	0x100000000
#define CONFIG_SYS_FSL_DCSR_BASE	0x700000000
#define CONFIG_SYS_FSL_DCSR_SIZE	0x40000000
#define CONFIG_SYS_FSL_MC_BASE		0x80c000000
#define CONFIG_SYS_FSL_MC_SIZE		0x4000000
#define CONFIG_SYS_FSL_NI_BASE		0x810000000
#define CONFIG_SYS_FSL_NI_SIZE		0x8000000
#define CONFIG_SYS_FSL_QBMAN_BASE	0x818000000
#define CONFIG_SYS_FSL_QBMAN_SIZE	0x8000000
#define CONFIG_SYS_FSL_QBMAN_SIZE_1	0x4000000
#define CONFIG_SYS_PCIE1_PHYS_SIZE	0x200000000
#define CONFIG_SYS_PCIE2_PHYS_SIZE	0x200000000
#define CONFIG_SYS_PCIE3_PHYS_SIZE	0x200000000
#define CONFIG_SYS_PCIE4_PHYS_SIZE	0x200000000
#define CONFIG_SYS_FSL_WRIOP1_BASE	0x4300000000
#define CONFIG_SYS_FSL_WRIOP1_SIZE	0x100000000
#define CONFIG_SYS_FSL_AIOP1_BASE	0x4b00000000
#define CONFIG_SYS_FSL_AIOP1_SIZE	0x100000000
#define CONFIG_SYS_FSL_PEBUF_BASE	0x4c00000000
#define CONFIG_SYS_FSL_PEBUF_SIZE	0x400000000
#define CONFIG_SYS_FSL_DRAM_BASE2	0x8080000000
#define CONFIG_SYS_FSL_DRAM_SIZE2	0x7F80000000

struct sys_mmu_table {
	u64 virt_addr;
	u64 phys_addr;
	u64 size;
	u64 memory_type;
	u64 share;
};

static const struct sys_mmu_table lsch3_early_mmu_table[] = {
	{ CONFIG_SYS_FSL_CCSR_BASE, CONFIG_SYS_FSL_CCSR_BASE,
	  CONFIG_SYS_FSL_CCSR_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_OCRAM_BASE, CONFIG_SYS_FSL_OCRAM_BASE,
	  CONFIG_SYS_FSL_OCRAM_SIZE, MT_NORMAL, PMD_SECT_NON_SHARE },
	/* For IFC Region #1, only the first 4MB is cache-enabled */
	{ CONFIG_SYS_FSL_IFC_BASE1, CONFIG_SYS_FSL_IFC_BASE1,
	  CONFIG_SYS_FSL_IFC_SIZE1_1, MT_NORMAL, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_IFC_BASE1 + CONFIG_SYS_FSL_IFC_SIZE1_1,
	  CONFIG_SYS_FSL_IFC_BASE1 + CONFIG_SYS_FSL_IFC_SIZE1_1,
	  CONFIG_SYS_FSL_IFC_SIZE1 - CONFIG_SYS_FSL_IFC_SIZE1_1,
	  MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FSL_IFC_BASE1,
	  CONFIG_SYS_FSL_IFC_SIZE1, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1, MT_NORMAL, PMD_SECT_OUTER_SHARE },
	{ CONFIG_SYS_FSL_DCSR_BASE, CONFIG_SYS_FSL_DCSR_BASE,
	  CONFIG_SYS_FSL_DCSR_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2, MT_NORMAL, PMD_SECT_OUTER_SHARE },
};

static const struct sys_mmu_table lsch3_final_mmu_table[] = {
	{ CONFIG_SYS_FSL_CCSR_BASE, CONFIG_SYS_FSL_CCSR_BASE,
	  CONFIG_SYS_FSL_CCSR_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_OCRAM_BASE, CONFIG_SYS_FSL_OCRAM_BASE,
	  CONFIG_SYS_FSL_OCRAM_SIZE, MT_NORMAL, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1, MT_NORMAL, PMD_SECT_OUTER_SHARE },
	{ CONFIG_SYS_FSL_QSPI_BASE2, CONFIG_SYS_FSL_QSPI_BASE2,
	  CONFIG_SYS_FSL_QSPI_SIZE2, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_IFC_BASE2, CONFIG_SYS_FSL_IFC_BASE2,
	  CONFIG_SYS_FSL_IFC_SIZE2, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_DCSR_BASE, CONFIG_SYS_FSL_DCSR_BASE,
	  CONFIG_SYS_FSL_DCSR_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_MC_BASE, CONFIG_SYS_FSL_MC_BASE,
	  CONFIG_SYS_FSL_MC_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_NI_BASE, CONFIG_SYS_FSL_NI_BASE,
	  CONFIG_SYS_FSL_NI_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	/* For QBMAN portal, only the first 64MB is cache-enabled */
	{ CONFIG_SYS_FSL_QBMAN_BASE, CONFIG_SYS_FSL_QBMAN_BASE,
	  CONFIG_SYS_FSL_QBMAN_SIZE_1, MT_NORMAL, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_QBMAN_BASE + CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  CONFIG_SYS_FSL_QBMAN_BASE + CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  CONFIG_SYS_FSL_QBMAN_SIZE - CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_PCIE1_PHYS_ADDR, CONFIG_SYS_PCIE1_PHYS_ADDR,
	  CONFIG_SYS_PCIE1_PHYS_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_PCIE2_PHYS_ADDR, CONFIG_SYS_PCIE2_PHYS_ADDR,
	  CONFIG_SYS_PCIE2_PHYS_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_PCIE3_PHYS_ADDR, CONFIG_SYS_PCIE3_PHYS_ADDR,
	  CONFIG_SYS_PCIE3_PHYS_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
#ifdef CONFIG_LS2085A
	{ CONFIG_SYS_PCIE4_PHYS_ADDR, CONFIG_SYS_PCIE4_PHYS_ADDR,
	  CONFIG_SYS_PCIE4_PHYS_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
#endif
	{ CONFIG_SYS_FSL_WRIOP1_BASE, CONFIG_SYS_FSL_WRIOP1_BASE,
	  CONFIG_SYS_FSL_WRIOP1_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_AIOP1_BASE, CONFIG_SYS_FSL_AIOP1_BASE,
	  CONFIG_SYS_FSL_AIOP1_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_PEBUF_BASE, CONFIG_SYS_FSL_PEBUF_BASE,
	  CONFIG_SYS_FSL_PEBUF_SIZE, MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE },
	{ CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2, MT_NORMAL, PMD_SECT_OUTER_SHARE },
};

struct table_info {
	u64 *ptr;
	u64 table_base;
	u64 entry_size;
};

/*
 * Set the block entries according to the information of the table.
 */
static int set_block_entry(const struct sys_mmu_table *list,
			   struct table_info *table)
{
	u64 block_size = 0, block_shift = 0;
	u64 block_addr, index;
	int j;

	if (table->entry_size == BLOCK_SIZE_L1) {
		block_size = BLOCK_SIZE_L1;
		block_shift = SECTION_SHIFT_L1;
	} else if (table->entry_size == BLOCK_SIZE_L2) {
		block_size = BLOCK_SIZE_L2;
		block_shift = SECTION_SHIFT_L2;
	} else {
		return -EINVAL;
	}

	block_addr = list->phys_addr;
	index = (list->virt_addr - table->table_base) >> block_shift;

	for (j = 0; j < (list->size >> block_shift); j++) {
		set_pgtable_section(table->ptr,
				    index,
				    block_addr,
				    list->memory_type,
				    list->share);
		block_addr += block_size;
		index++;
	}

	return 0;
}

/*
 * Find the corresponding table entry for the list.
 */
static int find_table(const struct sys_mmu_table *list,
		      struct table_info *table, u64 *level0_table)
{
	u64 index = 0, level = 0;
	u64 *level_table = level0_table;
	u64 temp_base = 0, block_size = 0, block_shift = 0;

	while (level < 3) {
		if (level == 0) {
			block_size = BLOCK_SIZE_L0;
			block_shift = SECTION_SHIFT_L0;
		} else if (level == 1) {
			block_size = BLOCK_SIZE_L1;
			block_shift = SECTION_SHIFT_L1;
		} else if (level == 2) {
			block_size = BLOCK_SIZE_L2;
			block_shift = SECTION_SHIFT_L2;
		}

		index = 0;
		while (list->virt_addr >= temp_base) {
			index++;
			temp_base += block_size;
		}

		temp_base -= block_size;

		if ((level_table[index - 1] & PMD_TYPE_MASK) ==
		    PMD_TYPE_TABLE) {
			level_table = (u64 *)(level_table[index - 1] &
				      ~PMD_TYPE_MASK);
			level++;
			continue;
		} else {
			if (level == 0)
				return -EINVAL;

			if ((list->phys_addr + list->size) >
			    (temp_base + block_size * NUM_OF_ENTRY))
				return -EINVAL;

			/*
			 * Check the address and size of the list member is
			 * aligned with the block size.
			 */
			if (((list->phys_addr & (block_size - 1)) != 0) ||
			    ((list->size & (block_size - 1)) != 0))
				return -EINVAL;

			table->ptr = level_table;
			table->table_base = temp_base -
					    ((index - 1) << block_shift);
			table->entry_size = block_size;

			return 0;
		}
	}
	return -EINVAL;
}

/*
 * To start MMU before DDR is available, we create MMU table in SRAM.
 * The base address of SRAM is CONFIG_SYS_FSL_OCRAM_BASE. We use three
 * levels of translation tables here to cover 40-bit address space.
 * We use 4KB granule size, with 40 bits physical address, T0SZ=24
 * Level 0 IA[39], table address @0
 * Level 1 IA[38:30], table address @0x1000, 0x2000
 * Level 2 IA[29:21], table address @0x3000, 0x4000
 * Address above 0x5000 is free for other purpose.
 */
static inline void early_mmu_setup(void)
{
	unsigned int el, i;
	u64 *level0_table = (u64 *)CONFIG_SYS_FSL_OCRAM_BASE;
	u64 *level1_table0 = (u64 *)(CONFIG_SYS_FSL_OCRAM_BASE + 0x1000);
	u64 *level1_table1 = (u64 *)(CONFIG_SYS_FSL_OCRAM_BASE + 0x2000);
	u64 *level2_table0 = (u64 *)(CONFIG_SYS_FSL_OCRAM_BASE + 0x3000);
	u64 *level2_table1 = (u64 *)(CONFIG_SYS_FSL_OCRAM_BASE + 0x4000);
	struct table_info table = {level0_table, 0, BLOCK_SIZE_L0};

	/* Invalidate all table entries */
	memset(level0_table, 0, 0x5000);

	/* Fill in the table entries */
	set_pgtable_table(level0_table, 0, level1_table0);
	set_pgtable_table(level0_table, 1, level1_table1);
	set_pgtable_table(level1_table0, 0, level2_table0);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FLASH_BASE >> SECTION_SHIFT_L1,
			  level2_table1);

	/* Find the table and fill in the block entries */
	for (i = 0; i < ARRAY_SIZE(lsch3_early_mmu_table); i++) {
		if (find_table(&lsch3_early_mmu_table[i],
			       &table, level0_table) == 0) {
			/*
			 * If find_table() returns error, it cannot be dealt
			 * with here. Breakpoint can be added for debugging.
			 */
			set_block_entry(&lsch3_early_mmu_table[i], &table);
			/*
			 * If set_block_entry() returns error, it cannot be
			 * dealt with here too.
			 */
		}
	}

	el = current_el();
	set_ttbr_tcr_mair(el, (u64)level0_table, LSCH3_TCR, MEMORY_ATTRIBUTES);
	set_sctlr(get_sctlr() | CR_M);
}

/*
 * The final tables look similar to early tables, but different in detail.
 * These tables are in DRAM. Sub tables are added to enable cache for
 * QBMan and OCRAM.
 *
 * Level 1 table 0 contains 512 entries for each 1GB from 0 to 512GB.
 * Level 1 table 1 contains 512 entries for each 1GB from 512GB to 1TB.
 * Level 2 table 0 contains 512 entries for each 2MB from 0 to 1GB.
 * Level 2 table 1 contains 512 entries for each 2MB from 32GB to 33GB.
 */
static inline void final_mmu_setup(void)
{
	unsigned int el, i;
	u64 *level0_table = (u64 *)gd->arch.tlb_addr;
	u64 *level1_table0 = (u64 *)(gd->arch.tlb_addr + 0x1000);
	u64 *level1_table1 = (u64 *)(gd->arch.tlb_addr + 0x2000);
	u64 *level2_table0 = (u64 *)(gd->arch.tlb_addr + 0x3000);
	u64 *level2_table1 = (u64 *)(gd->arch.tlb_addr + 0x4000);
	struct table_info table = {level0_table, 0, BLOCK_SIZE_L0};

	/* Invalidate all table entries */
	memset(level0_table, 0, PGTABLE_SIZE);

	/* Fill in the table entries */
	set_pgtable_table(level0_table, 0, level1_table0);
	set_pgtable_table(level0_table, 1, level1_table1);
	set_pgtable_table(level1_table0, 0, level2_table0);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_QBMAN_BASE >> SECTION_SHIFT_L1,
			  level2_table1);

	/* Find the table and fill in the block entries */
	for (i = 0; i < ARRAY_SIZE(lsch3_final_mmu_table); i++) {
		if (find_table(&lsch3_final_mmu_table[i],
			       &table, level0_table) == 0) {
			if (set_block_entry(&lsch3_final_mmu_table[i],
					    &table) != 0) {
				printf("MMU error: could not set block entry for %p\n",
				       &lsch3_final_mmu_table[i]);
			}

		} else {
			printf("MMU error: could not find the table for %p\n",
			       &lsch3_final_mmu_table[i]);
		}
	}

	/* flush new MMU table */
	flush_dcache_range(gd->arch.tlb_addr,
			   gd->arch.tlb_addr + gd->arch.tlb_size);

	/* point TTBR to the new table */
	el = current_el();
	set_ttbr_tcr_mair(el, (u64)level0_table, LSCH3_TCR_FINAL,
			  MEMORY_ATTRIBUTES);
	/*
	 * MMU is already enabled, just need to invalidate TLB to load the
	 * new table. The new table is compatible with the current table, if
	 * MMU somehow walks through the new table before invalidation TLB,
	 * it still works. So we don't need to turn off MMU here.
	 */
}

int arch_cpu_init(void)
{
	icache_enable();
	__asm_invalidate_dcache_all();
	__asm_invalidate_tlb_all();
	early_mmu_setup();
	set_sctlr(get_sctlr() | CR_C);
	return 0;
}

/*
 * This function is called from lib/board.c.
 * It recreates MMU table in main memory. MMU and d-cache are enabled earlier.
 * There is no need to disable d-cache for this operation.
 */
void enable_caches(void)
{
	final_mmu_setup();
	__asm_invalidate_tlb_all();
}
#endif

static inline u32 initiator_type(u32 cluster, int init_id)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 idx = (cluster >> (init_id * 8)) & TP_CLUSTER_INIT_MASK;
	u32 type = in_le32(&gur->tp_ityp[idx]);

	if (type & TP_ITYP_AV)
		return type;

	return 0;
}

u32 cpu_mask(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type, mask = 0;

	do {
		int j;
		cluster = in_le32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
					mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return mask;
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	return hweight32(cpu_mask());
}

int fsl_qoriq_core_to_cluster(unsigned int core)
{
	struct ccsr_gur __iomem *gur =
		(void __iomem *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster;

	do {
		int j;
		cluster = in_le32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			if (initiator_type(cluster, j)) {
				if (count == core)
					return i;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return -1;      /* cannot identify the cluster */
}

u32 fsl_qoriq_core_to_type(unsigned int core)
{
	struct ccsr_gur __iomem *gur =
		(void __iomem *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type;

	do {
		int j;
		cluster = in_le32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (count == core)
					return type;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return -1;      /* cannot identify the cluster */
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct sys_info sysinfo;
	char buf[32];
	unsigned int i, core;
	u32 type;

	puts("SoC: ");

	cpu_name(buf);
	printf(" %s (0x%x)\n", buf, in_le32(&gur->svr));

	memset((u8 *)buf, 0x00, ARRAY_SIZE(buf));

	get_sys_info(&sysinfo);
	puts("Clock Configuration:");
	for_each_cpu(i, core, cpu_numcores(), cpu_mask()) {
		if (!(i % 3))
			puts("\n       ");
		type = TP_ITYP_VER(fsl_qoriq_core_to_type(core));
		printf("CPU%d(%s):%-4s MHz  ", core,
		       type == TY_ITYP_VER_A7 ? "A7 " :
		       (type == TY_ITYP_VER_A53 ? "A53" :
			(type == TY_ITYP_VER_A57 ? "A57" : "   ")),
		       strmhz(buf, sysinfo.freq_processor[core]));
	}
	printf("\n       Bus:      %-4s MHz  ",
	       strmhz(buf, sysinfo.freq_systembus));
	printf("DDR:      %-4s MT/s", strmhz(buf, sysinfo.freq_ddrbus));
	printf("     DP-DDR:   %-4s MT/s", strmhz(buf, sysinfo.freq_ddrbus2));
	puts("\n");

	/* Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		u32 rcw = in_le32(&gur->rcwsr[i]);

		if ((i % 4) == 0)
			printf("\n       %02x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int error = 0;

#ifdef CONFIG_FSL_MC_ENET
	error = fsl_mc_ldpaa_init(bis);
#endif
	return error;
}

int arch_early_init_r(void)
{
	int rv;
	rv = fsl_lsch3_wake_seconday_cores();

	if (rv)
		printf("Did not wake secondary cores\n");

#ifdef CONFIG_SYS_HAS_SERDES
	fsl_serdes_init();
#endif
	return 0;
}

int timer_init(void)
{
	u32 __iomem *cntcr = (u32 *)CONFIG_SYS_FSL_TIMER_ADDR;
	u32 __iomem *cltbenr = (u32 *)CONFIG_SYS_FSL_PMU_CLTBENR;
#ifdef COUNTER_FREQUENCY_REAL
	unsigned long cntfrq = COUNTER_FREQUENCY_REAL;

	/* Update with accurate clock frequency */
	asm volatile("msr cntfrq_el0, %0" : : "r" (cntfrq) : "memory");
#endif

	/* Enable timebase for all clusters.
	 * It is safe to do so even some clusters are not enabled.
	 */
	out_le32(cltbenr, 0xf);

	/* Enable clock for timer
	 * This is a global setting.
	 */
	out_le32(cntcr, 0x1);

	return 0;
}

void reset_cpu(ulong addr)
{
	u32 __iomem *rstcr = (u32 *)CONFIG_SYS_FSL_RST_ADDR;
	u32 val;

	/* Raise RESET_REQ_B */
	val = in_le32(rstcr);
	val |= 0x02;
	out_le32(rstcr, val);
}
