/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_DCACHE_OFF

#ifdef CONFIG_SYS_FULL_VA
static void set_ptl1_entry(u64 index, u64 ptl2_entry)
{
	u64 *pgd = (u64 *)gd->arch.tlb_addr;
	u64 value;

	value = ptl2_entry | PTL1_TYPE_TABLE;
	pgd[index] = value;
}

static void set_ptl2_block(u64 ptl1, u64 bfn, u64 address, u64 memory_attrs)
{
	u64 *pmd = (u64 *)ptl1;
	u64 value;

	value = address | PTL2_TYPE_BLOCK | PTL2_BLOCK_AF;
	value |= memory_attrs;
	pmd[bfn] = value;
}

static struct mm_region mem_map[] = CONFIG_SYS_MEM_MAP;

#define PTL1_ENTRIES CONFIG_SYS_PTL1_ENTRIES
#define PTL2_ENTRIES CONFIG_SYS_PTL2_ENTRIES

static void setup_pgtables(void)
{
	int l1_e, l2_e;
	unsigned long pmd = 0;
	unsigned long address;

	/* Setup the PMD pointers */
	for (l1_e = 0; l1_e < CONFIG_SYS_MEM_MAP_SIZE; l1_e++) {
		gd->arch.pmd_addr[l1_e] = gd->arch.tlb_addr +
						PTL1_ENTRIES * sizeof(u64);
		gd->arch.pmd_addr[l1_e] += PTL2_ENTRIES * sizeof(u64) * l1_e;
		gd->arch.pmd_addr[l1_e] = ALIGN(gd->arch.pmd_addr[l1_e],
						0x10000UL);
	}

	/* Setup the page tables */
	for (l1_e = 0; l1_e < PTL1_ENTRIES; l1_e++) {
		if (mem_map[pmd].base ==
			(uintptr_t)l1_e << PTL2_BITS) {
			set_ptl1_entry(l1_e, gd->arch.pmd_addr[pmd]);

			for (l2_e = 0; l2_e < PTL2_ENTRIES; l2_e++) {
				address = mem_map[pmd].base
					+ (uintptr_t)l2_e * BLOCK_SIZE;
				set_ptl2_block(gd->arch.pmd_addr[pmd], l2_e,
					       address, mem_map[pmd].attrs);
			}

			pmd++;
		} else {
			set_ptl1_entry(l1_e, 0);
		}
	}
}

#else

inline void set_pgtable_section(u64 *page_table, u64 index, u64 section,
			 u64 memory_type, u64 attribute)
{
	u64 value;

	value = section | PMD_TYPE_SECT | PMD_SECT_AF;
	value |= PMD_ATTRINDX(memory_type);
	value |= attribute;
	page_table[index] = value;
}

inline void set_pgtable_table(u64 *page_table, u64 index, u64 *table_addr)
{
	u64 value;

	value = (u64)table_addr | PMD_TYPE_TABLE;
	page_table[index] = value;
}
#endif

/* to activate the MMU we need to set up virtual memory */
__weak void mmu_setup(void)
{
#ifndef CONFIG_SYS_FULL_VA
	bd_t *bd = gd->bd;
	u64 *page_table = (u64 *)gd->arch.tlb_addr, i, j;
#endif
	int el;

#ifdef CONFIG_SYS_FULL_VA
	unsigned long coreid = read_mpidr() & CONFIG_COREID_MASK;

	/* Set up page tables only on BSP */
	if (coreid == BSP_COREID)
		setup_pgtables();
#else
	/* Setup an identity-mapping for all spaces */
	for (i = 0; i < (PGTABLE_SIZE >> 3); i++) {
		set_pgtable_section(page_table, i, i << SECTION_SHIFT,
				    MT_DEVICE_NGNRNE, PMD_SECT_NON_SHARE);
	}

	/* Setup an identity-mapping for all RAM space */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ulong start = bd->bi_dram[i].start;
		ulong end = bd->bi_dram[i].start + bd->bi_dram[i].size;
		for (j = start >> SECTION_SHIFT;
		     j < end >> SECTION_SHIFT; j++) {
			set_pgtable_section(page_table, j, j << SECTION_SHIFT,
					    MT_NORMAL, PMD_SECT_NON_SHARE);
		}
	}

#endif
	/* load TTBR0 */
	el = current_el();
	if (el == 1) {
		set_ttbr_tcr_mair(el, gd->arch.tlb_addr,
				  TCR_EL1_RSVD | TCR_FLAGS | TCR_EL1_IPS_BITS,
				  MEMORY_ATTRIBUTES);
	} else if (el == 2) {
		set_ttbr_tcr_mair(el, gd->arch.tlb_addr,
				  TCR_EL2_RSVD | TCR_FLAGS | TCR_EL2_IPS_BITS,
				  MEMORY_ATTRIBUTES);
	} else {
		set_ttbr_tcr_mair(el, gd->arch.tlb_addr,
				  TCR_EL3_RSVD | TCR_FLAGS | TCR_EL3_IPS_BITS,
				  MEMORY_ATTRIBUTES);
	}
	/* enable the mmu */
	set_sctlr(get_sctlr() | CR_M);
}

/*
 * Performs a invalidation of the entire data cache at all levels
 */
void invalidate_dcache_all(void)
{
	__asm_invalidate_dcache_all();
}

/*
 * Performs a clean & invalidation of the entire data cache at all levels.
 * This function needs to be inline to avoid using stack.
 * __asm_flush_l3_cache return status of timeout
 */
inline void flush_dcache_all(void)
{
	int ret;

	__asm_flush_dcache_all();
	ret = __asm_flush_l3_cache();
	if (ret)
		debug("flushing dcache returns 0x%x\n", ret);
	else
		debug("flushing dcache successfully.\n");
}

/*
 * Invalidates range in all levels of D-cache/unified cache
 */
void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	__asm_flush_dcache_range(start, stop);
}

/*
 * Flush range(clean & invalidate) from all levels of D-cache/unified cache
 */
void flush_dcache_range(unsigned long start, unsigned long stop)
{
	__asm_flush_dcache_range(start, stop);
}

void dcache_enable(void)
{
	/* The data cache is not active unless the mmu is enabled */
	if (!(get_sctlr() & CR_M)) {
		invalidate_dcache_all();
		__asm_invalidate_tlb_all();
		mmu_setup();
	}

	set_sctlr(get_sctlr() | CR_C);
}

void dcache_disable(void)
{
	uint32_t sctlr;

	sctlr = get_sctlr();

	/* if cache isn't enabled no need to disable */
	if (!(sctlr & CR_C))
		return;

	set_sctlr(sctlr & ~(CR_C|CR_M));

	flush_dcache_all();
	__asm_invalidate_tlb_all();
}

int dcache_status(void)
{
	return (get_sctlr() & CR_C) != 0;
}

u64 *__weak arch_get_page_table(void) {
	puts("No page table offset defined\n");

	return NULL;
}

#ifndef CONFIG_SYS_FULL_VA
void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
	u64 *page_table = arch_get_page_table();
	u64 upto, end;

	if (page_table == NULL)
		return;

	end = ALIGN(start + size, (1 << MMU_SECTION_SHIFT)) >>
	      MMU_SECTION_SHIFT;
	start = start >> MMU_SECTION_SHIFT;
	for (upto = start; upto < end; upto++) {
		page_table[upto] &= ~PMD_ATTRINDX_MASK;
		page_table[upto] |= PMD_ATTRINDX(option);
	}
	asm volatile("dsb sy");
	__asm_invalidate_tlb_all();
	asm volatile("dsb sy");
	asm volatile("isb");
	start = start << MMU_SECTION_SHIFT;
	end = end << MMU_SECTION_SHIFT;
	flush_dcache_range(start, end);
	asm volatile("dsb sy");
}
#endif

#else	/* CONFIG_SYS_DCACHE_OFF */

void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

int dcache_status(void)
{
	return 0;
}

void mmu_set_region_dcache_behaviour(phys_addr_t start, size_t size,
				     enum dcache_option option)
{
}

#endif	/* CONFIG_SYS_DCACHE_OFF */

#ifndef CONFIG_SYS_ICACHE_OFF

void icache_enable(void)
{
	__asm_invalidate_icache_all();
	set_sctlr(get_sctlr() | CR_I);
}

void icache_disable(void)
{
	set_sctlr(get_sctlr() & ~CR_I);
}

int icache_status(void)
{
	return (get_sctlr() & CR_I) != 0;
}

void invalidate_icache_all(void)
{
	__asm_invalidate_icache_all();
}

#else	/* CONFIG_SYS_ICACHE_OFF */

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

void invalidate_icache_all(void)
{
}

#endif	/* CONFIG_SYS_ICACHE_OFF */

/*
 * Enable dCache & iCache, whether cache is actually enabled
 * depend on CONFIG_SYS_DCACHE_OFF and CONFIG_SYS_ICACHE_OFF
 */
void __weak enable_caches(void)
{
	icache_enable();
	dcache_enable();
}
