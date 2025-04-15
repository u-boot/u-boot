/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#ifndef _ASM_ARMV8_MMU_H_
#define _ASM_ARMV8_MMU_H_

#include <hang.h>
#include <linux/const.h>

/*
 * block/section address mask and size definitions.
 */

/* PAGE_SHIFT determines the page size */
#undef  PAGE_SIZE
#define PAGE_SHIFT		12
#define PAGE_SIZE		(1 << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE - 1))

/***************************************************************/

/*
 * Memory types
 */
#define MT_DEVICE_NGNRNE	0
#define MT_DEVICE_NGNRE		1
#define MT_DEVICE_GRE		2
#define MT_NORMAL_NC		3
#define MT_NORMAL		4

#define MEMORY_ATTRIBUTES	((0x00 << (MT_DEVICE_NGNRNE * 8)) |	\
				(0x04 << (MT_DEVICE_NGNRE * 8))   |	\
				(0x0c << (MT_DEVICE_GRE * 8))     |	\
				(0x44 << (MT_NORMAL_NC * 8))      |	\
				(UL(0xff) << (MT_NORMAL * 8)))

/*
 * Hardware page table definitions.
 *
 */

#define PTE_TYPE_MASK		(3 << 0)
#define PTE_TYPE_FAULT		(0 << 0)
#define PTE_TYPE_TABLE		(3 << 0)
#define PTE_TYPE_PAGE		(3 << 0)
#define PTE_TYPE_BLOCK		(1 << 0)
#define PTE_TYPE_VALID		(1 << 0)

#define PTE_TABLE_PXN		(1UL << 59)
#define PTE_TABLE_XN		(1UL << 60)
#define PTE_TABLE_AP		(3UL << 61)
#define PTE_TABLE_NS		(1UL << 63)

/*
 * Block
 */
#define PTE_BLOCK_MEMTYPE(x)	((x) << 2)
#define PTE_BLOCK_NS            (1 << 5)
#define PTE_BLOCK_NON_SHARE	(0 << 8)
#define PTE_BLOCK_OUTER_SHARE	(2 << 8)
#define PTE_BLOCK_INNER_SHARE	(3 << 8)
#define PTE_BLOCK_AF		(1 << 10)
#define PTE_BLOCK_NG		(1 << 11)
#define PTE_BLOCK_PXN		(UL(1) << 53)
#define PTE_BLOCK_UXN		(UL(1) << 54)
#define PTE_BLOCK_RO            (UL(1) << 7)

/*
 * AttrIndx[2:0]
 */
#define PMD_ATTRINDX(t)		((t) << 2)
#define PMD_ATTRINDX_MASK	(7 << 2)
#define PMD_ATTRMASK		(PTE_BLOCK_PXN		| \
				 PTE_BLOCK_UXN		| \
				 PMD_ATTRINDX_MASK	| \
				 PTE_BLOCK_RO		| \
				 PTE_TYPE_VALID)

/*
 * TCR flags.
 */
#define TCR_T0SZ(x)		((64 - (x)) << 0)
#define TCR_IRGN_NC		(0 << 8)
#define TCR_IRGN_WBWA		(1 << 8)
#define TCR_IRGN_WT		(2 << 8)
#define TCR_IRGN_WBNWA		(3 << 8)
#define TCR_IRGN_MASK		(3 << 8)
#define TCR_ORGN_NC		(0 << 10)
#define TCR_ORGN_WBWA		(1 << 10)
#define TCR_ORGN_WT		(2 << 10)
#define TCR_ORGN_WBNWA		(3 << 10)
#define TCR_ORGN_MASK		(3 << 10)
#define TCR_SHARED_NON		(0 << 12)
#define TCR_SHARED_OUTER	(2 << 12)
#define TCR_SHARED_INNER	(3 << 12)
#define TCR_TG0_4K		(0 << 14)
#define TCR_TG0_64K		(1 << 14)
#define TCR_TG0_16K		(2 << 14)
#define TCR_EPD1_DISABLE	(1 << 23)

#define TCR_EL1_RSVD		(1U << 31)
#define TCR_EL2_RSVD		(1U << 31 | 1 << 23)
#define TCR_EL3_RSVD		(1U << 31 | 1 << 23)

#define HCR_EL2_E2H_BIT		34

#ifndef __ASSEMBLY__
#include <linux/types.h>

static inline void set_ttbr_tcr_mair(int el, u64 table, u64 tcr, u64 attr)
{
	asm volatile("dsb sy");
	if (el == 1) {
		asm volatile("msr ttbr0_el1, %0" : : "r" (table) : "memory");
		asm volatile("msr tcr_el1, %0" : : "r" (tcr) : "memory");
		asm volatile("msr mair_el1, %0" : : "r" (attr) : "memory");
	} else if (el == 2) {
		asm volatile("msr ttbr0_el2, %0" : : "r" (table) : "memory");
		asm volatile("msr tcr_el2, %0" : : "r" (tcr) : "memory");
		asm volatile("msr mair_el2, %0" : : "r" (attr) : "memory");
	} else if (el == 3) {
		asm volatile("msr ttbr0_el3, %0" : : "r" (table) : "memory");
		asm volatile("msr tcr_el3, %0" : : "r" (tcr) : "memory");
		asm volatile("msr mair_el3, %0" : : "r" (attr) : "memory");
	} else {
		hang();
	}
	asm volatile("isb");
}

static inline void get_ttbr_tcr_mair(int el, u64 *table, u64 *tcr, u64 *attr)
{
	if (el == 1) {
		asm volatile("mrs %0, ttbr0_el1" : "=r" (*table));
		asm volatile("mrs %0, tcr_el1" : "=r" (*tcr));
		asm volatile("mrs %0, mair_el1" : "=r" (*attr));
	} else if (el == 2) {
		asm volatile("mrs %0, ttbr0_el2" : "=r" (*table));
		asm volatile("mrs %0, tcr_el2" : "=r" (*tcr));
		asm volatile("mrs %0, mair_el2" : "=r" (*attr));
	} else if (el == 3) {
		asm volatile("mrs %0, ttbr0_el3" : "=r" (*table));
		asm volatile("mrs %0, tcr_el3" : "=r" (*tcr));
		asm volatile("mrs %0, mair_el3" : "=r" (*attr));
	} else {
		hang();
	}
}

/**
 * typedef pte_walker_cb_t - callback function for walk_pagetable.
 *
 * This function is called when the walker finds a table entry
 * or after parsing a block or pages. For a table the @end address
 * is 0, and @addr is the address of the table. Otherwise, they
 * are the start and end physical addresses of the block or page.
 *
 * @addr: PTE start address (PA), or address of table. Includes attributes.
 * @end: End address of the region (or 0 for a table)
 * @va_bits: Number of bits in the virtual address
 * @level: Table level
 * @priv: Private data for the callback
 *
 * Return: true to stop walking, false to continue
 */
typedef bool (*pte_walker_cb_t)(u64 addr, u64 end, int va_bits, int level, void *priv);

/**
 * walk_pagetable() - Walk the pagetable at ttbr and call @cb for each region
 *
 * @ttbr: Address of the pagetable to dump
 * @tcr: TCR value to use
 * @cb: Callback function to call for each entry
 * @priv: Private data for the callback
 */
void walk_pagetable(u64 ttbr, u64 tcr, pte_walker_cb_t cb, void *priv);

/**
 * dump_pagetable() - Dump the pagetable at ttbr, printing each region and
 * level.
 *
 * @ttbr: Address of the pagetable to dump
 * @tcr: TCR value to use
 */
void dump_pagetable(u64 ttbr, u64 tcr);

struct mm_region {
	u64 virt;
	u64 phys;
	u64 size;
	u64 attrs;
};

extern struct mm_region *mem_map;
void setup_pgtables(void);
u64 get_tcr(u64 *pips, u64 *pva_bits);
#endif

#endif /* _ASM_ARMV8_MMU_H_ */
