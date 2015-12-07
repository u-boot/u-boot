/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/arch/cpu.h>
#include <asm/arch/speed.h>
#ifdef CONFIG_MP
#include <asm/arch/mp.h>
#endif
#include <fm_eth.h>
#include <fsl_debug_server.h>
#include <fsl-mc/fsl_mc.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

void cpu_name(char *name)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int i, svr, ver;

	svr = gur_in32(&gur->svr);
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
				    list->attribute);
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

#ifdef CONFIG_FSL_LSCH3
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FLASH_BASE >> SECTION_SHIFT_L1,
			  level2_table1);
#elif defined(CONFIG_FSL_LSCH2)
	set_pgtable_table(level1_table0, 1, level2_table1);
#endif
	/* Find the table and fill in the block entries */
	for (i = 0; i < ARRAY_SIZE(early_mmu_table); i++) {
		if (find_table(&early_mmu_table[i],
			       &table, level0_table) == 0) {
			/*
			 * If find_table() returns error, it cannot be dealt
			 * with here. Breakpoint can be added for debugging.
			 */
			set_block_entry(&early_mmu_table[i], &table);
			/*
			 * If set_block_entry() returns error, it cannot be
			 * dealt with here too.
			 */
		}
	}

	el = current_el();

	set_ttbr_tcr_mair(el, (u64)level0_table, LAYERSCAPE_TCR,
			  MEMORY_ATTRIBUTES);
	set_sctlr(get_sctlr() | CR_M);
}

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
/*
 * Called from final mmu setup. The phys_addr is new, non-existing
 * address. A new sub table is created @level2_table_secure to cover
 * size of CONFIG_SYS_MEM_RESERVE_SECURE memory.
 */
static inline int final_secure_ddr(u64 *level0_table,
				   u64 *level2_table_secure,
				   phys_addr_t phys_addr)
{
	int ret = -EINVAL;
	struct table_info table = {};
	struct sys_mmu_table ddr_entry = {
		0, 0, BLOCK_SIZE_L1, MT_NORMAL,
		PMD_SECT_OUTER_SHARE | PMD_SECT_NS
	};
	u64 index;

	/* Need to create a new table */
	ddr_entry.virt_addr = phys_addr & ~(BLOCK_SIZE_L1 - 1);
	ddr_entry.phys_addr = phys_addr & ~(BLOCK_SIZE_L1 - 1);
	ret = find_table(&ddr_entry, &table, level0_table);
	if (ret)
		return ret;
	index = (ddr_entry.virt_addr - table.table_base) >> SECTION_SHIFT_L1;
	set_pgtable_table(table.ptr, index, level2_table_secure);
	table.ptr = level2_table_secure;
	table.table_base = ddr_entry.virt_addr;
	table.entry_size = BLOCK_SIZE_L2;
	ret = set_block_entry(&ddr_entry, &table);
	if (ret) {
		printf("MMU error: could not fill non-secure ddr block entries\n");
		return ret;
	}
	ddr_entry.virt_addr = phys_addr;
	ddr_entry.phys_addr = phys_addr;
	ddr_entry.size = CONFIG_SYS_MEM_RESERVE_SECURE;
	ddr_entry.attribute = PMD_SECT_OUTER_SHARE;
	ret = find_table(&ddr_entry, &table, level0_table);
	if (ret) {
		printf("MMU error: could not find secure ddr table\n");
		return ret;
	}
	ret = set_block_entry(&ddr_entry, &table);
	if (ret)
		printf("MMU error: could not set secure ddr block entry\n");

	return ret;
}
#endif

/*
 * The final tables look similar to early tables, but different in detail.
 * These tables are in DRAM. Sub tables are added to enable cache for
 * QBMan and OCRAM.
 *
 * Put the MMU table in secure memory if gd->secure_ram is valid.
 * OCRAM will be not used for this purpose so gd->secure_ram can't be 0.
 *
 * Level 1 table 0 contains 512 entries for each 1GB from 0 to 512GB.
 * Level 1 table 1 contains 512 entries for each 1GB from 512GB to 1TB.
 * Level 2 table 0 contains 512 entries for each 2MB from 0 to 1GB.
 *
 * For LSCH3:
 * Level 2 table 1 contains 512 entries for each 2MB from 32GB to 33GB.
 * For LSCH2:
 * Level 2 table 1 contains 512 entries for each 2MB from 1GB to 2GB.
 * Level 2 table 2 contains 512 entries for each 2MB from 20GB to 21GB.
 */
static inline void final_mmu_setup(void)
{
	unsigned int el = current_el();
	unsigned int i;
	u64 *level0_table = (u64 *)gd->arch.tlb_addr;
	u64 *level1_table0;
	u64 *level1_table1;
	u64 *level2_table0;
	u64 *level2_table1;
#ifdef CONFIG_FSL_LSCH2
	u64 *level2_table2;
#endif
	struct table_info table = {NULL, 0, BLOCK_SIZE_L0};

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	u64 *level2_table_secure;

	if (el == 3) {
		/*
		 * Only use gd->secure_ram if the address is recalculated
		 * Align to 4KB for MMU table
		 */
		if (gd->secure_ram & MEM_RESERVE_SECURE_MAINTAINED)
			level0_table = (u64 *)(gd->secure_ram & ~0xfff);
		else
			printf("MMU warning: gd->secure_ram is not maintained, disabled.\n");
	}
#endif
	level1_table0 = level0_table + 512;
	level1_table1 = level1_table0 + 512;
	level2_table0 = level1_table1 + 512;
	level2_table1 = level2_table0 + 512;
#ifdef CONFIG_FSL_LSCH2
	level2_table2 = level2_table1 + 512;
#endif
	table.ptr = level0_table;

	/* Invalidate all table entries */
	memset(level0_table, 0, PGTABLE_SIZE);

	/* Fill in the table entries */
	set_pgtable_table(level0_table, 0, level1_table0);
	set_pgtable_table(level0_table, 1, level1_table1);
	set_pgtable_table(level1_table0, 0, level2_table0);
#ifdef CONFIG_FSL_LSCH3
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_QBMAN_BASE >> SECTION_SHIFT_L1,
			  level2_table1);
#elif defined(CONFIG_FSL_LSCH2)
	set_pgtable_table(level1_table0, 1, level2_table1);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_QBMAN_BASE >> SECTION_SHIFT_L1,
			  level2_table2);
#endif

	/* Find the table and fill in the block entries */
	for (i = 0; i < ARRAY_SIZE(final_mmu_table); i++) {
		if (find_table(&final_mmu_table[i],
			       &table, level0_table) == 0) {
			if (set_block_entry(&final_mmu_table[i],
					    &table) != 0) {
				printf("MMU error: could not set block entry for %p\n",
				       &final_mmu_table[i]);
			}

		} else {
			printf("MMU error: could not find the table for %p\n",
			       &final_mmu_table[i]);
		}
	}
	/* Set the secure memory to secure in MMU */
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	if (el == 3 && gd->secure_ram & MEM_RESERVE_SECURE_MAINTAINED) {
#ifdef CONFIG_FSL_LSCH3
		level2_table_secure = level2_table1 + 512;
#elif defined(CONFIG_FSL_LSCH2)
		level2_table_secure = level2_table2 + 512;
#endif
		if (!final_secure_ddr(level0_table,
				      level2_table_secure,
				      gd->secure_ram & ~0x3)) {
			gd->secure_ram |= MEM_RESERVE_SECURE_SECURED;
			debug("Now MMU table is in secured memory at 0x%llx\n",
			      gd->secure_ram & ~0x3);
		} else {
			printf("MMU warning: Failed to secure DDR\n");
		}
	}
#endif

	/* flush new MMU table */
	flush_dcache_range((ulong)level0_table,
			   (ulong)level0_table + gd->arch.tlb_size);

#ifdef CONFIG_SYS_DPAA_FMAN
	flush_dcache_all();
#endif
	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, (u64)level0_table, LAYERSCAPE_TCR_FINAL,
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
	u32 type = 0;

	type = gur_in32(&gur->tp_ityp[idx]);
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

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
					mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

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

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			if (initiator_type(cluster, j)) {
				if (count == core)
					return i;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

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

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (count == core)
					return type;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return -1;      /* cannot identify the cluster */
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct sys_info sysinfo;
	char buf[32];
	unsigned int i, core;
	u32 type, rcw;

	puts("SoC: ");

	cpu_name(buf);
	printf(" %s (0x%x)\n", buf, gur_in32(&gur->svr));
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
#ifdef CONFIG_SYS_DPAA_FMAN
	printf("  FMAN:     %-4s MHz", strmhz(buf, sysinfo.freq_fman[0]));
#endif
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	printf("     DP-DDR:   %-4s MT/s", strmhz(buf, sysinfo.freq_ddrbus2));
#endif
	puts("\n");

	/*
	 * Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		rcw = gur_in32(&gur->rcwsr[i]);
		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
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
#ifdef CONFIG_FMAN_ENET
	fm_standard_init(bis);
#endif
	return error;
}

int arch_early_init_r(void)
{
#ifdef CONFIG_MP
	int rv = 1;
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A009635
	erratum_a009635();
#endif

#ifdef CONFIG_MP
	rv = fsl_layerscape_wake_seconday_cores();
	if (rv)
		printf("Did not wake secondary cores\n");
#endif

#ifdef CONFIG_SYS_HAS_SERDES
	fsl_serdes_init();
#endif
#ifdef CONFIG_FMAN_ENET
	fman_enet_init();
#endif
	return 0;
}

int timer_init(void)
{
	u32 __iomem *cntcr = (u32 *)CONFIG_SYS_FSL_TIMER_ADDR;
#ifdef CONFIG_FSL_LSCH3
	u32 __iomem *cltbenr = (u32 *)CONFIG_SYS_FSL_PMU_CLTBENR;
#endif
#ifdef COUNTER_FREQUENCY_REAL
	unsigned long cntfrq = COUNTER_FREQUENCY_REAL;

	/* Update with accurate clock frequency */
	asm volatile("msr cntfrq_el0, %0" : : "r" (cntfrq) : "memory");
#endif

#ifdef CONFIG_FSL_LSCH3
	/* Enable timebase for all clusters.
	 * It is safe to do so even some clusters are not enabled.
	 */
	out_le32(cltbenr, 0xf);
#endif

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
	val = scfg_in32(rstcr);
	val |= 0x02;
	scfg_out32(rstcr, val);
}

phys_size_t board_reserve_ram_top(phys_size_t ram_size)
{
	phys_size_t ram_top = ram_size;

#ifdef CONFIG_SYS_MEM_TOP_HIDE
#error CONFIG_SYS_MEM_TOP_HIDE not to be used together with this function
#endif
/* Carve the Debug Server private DRAM block from the end of DRAM */
#ifdef CONFIG_FSL_DEBUG_SERVER
	ram_top -= debug_server_get_dram_block_size();
#endif

/* Carve the MC private DRAM block from the end of DRAM */
#ifdef CONFIG_FSL_MC_ENET
	ram_top -= mc_get_dram_block_size();
	ram_top &= ~(CONFIG_SYS_MC_RSV_MEM_ALIGN - 1);
#endif

	return ram_top;
}
