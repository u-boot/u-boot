// SPDX-License-Identifier: GPL-2.0+
/*
 * Procedures for maintaining information about logical memory blocks.
 *
 * Peter Bergner, IBM Corp.	June 2001.
 * Copyright (C) 2001 Peter Bergner.
 */

#include <common.h>
#include <image.h>
#include <lmb.h>
#include <log.h>
#include <malloc.h>

#define LMB_ALLOC_ANYWHERE	0

static void lmb_dump_region(struct lmb_region *rgn, char *name)
{
	unsigned long long base, size, end;
	enum lmb_flags flags;
	int i;

	printf(" %s.cnt  = 0x%lx\n", name, rgn->cnt);

	for (i = 0; i < rgn->cnt; i++) {
		base = rgn->region[i].base;
		size = rgn->region[i].size;
		end = base + size - 1;
		flags = rgn->region[i].flags;

		printf(" %s[%d]\t[0x%llx-0x%llx], 0x%08llx bytes flags: %x\n",
		       name, i, base, end, size, flags);
	}
}

void lmb_dump_all_force(struct lmb *lmb)
{
	printf("lmb_dump_all:\n");
	lmb_dump_region(&lmb->memory, "memory");
	lmb_dump_region(&lmb->reserved, "reserved");
}

void lmb_dump_all(struct lmb *lmb)
{
#ifdef DEBUG
	lmb_dump_all_force(lmb);
#endif
}

static long lmb_addrs_overlap(phys_addr_t base1, phys_size_t size1,
			      phys_addr_t base2, phys_size_t size2)
{
	const phys_addr_t base1_end = base1 + size1 - 1;
	const phys_addr_t base2_end = base2 + size2 - 1;

	return ((base1 <= base2_end) && (base2 <= base1_end));
}

static long lmb_addrs_adjacent(phys_addr_t base1, phys_size_t size1,
			       phys_addr_t base2, phys_size_t size2)
{
	if (base2 == base1 + size1)
		return 1;
	else if (base1 == base2 + size2)
		return -1;

	return 0;
}

static long lmb_regions_adjacent(struct lmb_region *rgn, unsigned long r1,
				 unsigned long r2)
{
	phys_addr_t base1 = rgn->region[r1].base;
	phys_size_t size1 = rgn->region[r1].size;
	phys_addr_t base2 = rgn->region[r2].base;
	phys_size_t size2 = rgn->region[r2].size;

	return lmb_addrs_adjacent(base1, size1, base2, size2);
}

static void lmb_remove_region(struct lmb_region *rgn, unsigned long r)
{
	unsigned long i;

	for (i = r; i < rgn->cnt - 1; i++) {
		rgn->region[i].base = rgn->region[i + 1].base;
		rgn->region[i].size = rgn->region[i + 1].size;
		rgn->region[i].flags = rgn->region[i + 1].flags;
	}
	rgn->cnt--;
}

/* Assumption: base addr of region 1 < base addr of region 2 */
static void lmb_coalesce_regions(struct lmb_region *rgn, unsigned long r1,
				 unsigned long r2)
{
	rgn->region[r1].size += rgn->region[r2].size;
	lmb_remove_region(rgn, r2);
}

void lmb_init(struct lmb *lmb)
{
#if IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
	lmb->memory.max = CONFIG_LMB_MAX_REGIONS;
	lmb->reserved.max = CONFIG_LMB_MAX_REGIONS;
#else
	lmb->memory.max = CONFIG_LMB_MEMORY_REGIONS;
	lmb->reserved.max = CONFIG_LMB_RESERVED_REGIONS;
	lmb->memory.region = lmb->memory_regions;
	lmb->reserved.region = lmb->reserved_regions;
#endif
	lmb->memory.cnt = 0;
	lmb->reserved.cnt = 0;
}

static void lmb_reserve_common(struct lmb *lmb, void *fdt_blob)
{
	arch_lmb_reserve(lmb);
	board_lmb_reserve(lmb);

	if (IMAGE_ENABLE_OF_LIBFDT && fdt_blob)
		boot_fdt_add_mem_rsv_regions(lmb, fdt_blob);
}

/* Initialize the struct, add memory and call arch/board reserve functions */
void lmb_init_and_reserve(struct lmb *lmb, struct bd_info *bd, void *fdt_blob)
{
	int i;

	lmb_init(lmb);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (bd->bi_dram[i].size) {
			lmb_add(lmb, bd->bi_dram[i].start,
				bd->bi_dram[i].size);
		}
	}

	lmb_reserve_common(lmb, fdt_blob);
}

/* Initialize the struct, add memory and call arch/board reserve functions */
void lmb_init_and_reserve_range(struct lmb *lmb, phys_addr_t base,
				phys_size_t size, void *fdt_blob)
{
	lmb_init(lmb);
	lmb_add(lmb, base, size);
	lmb_reserve_common(lmb, fdt_blob);
}

/* This routine called with relocation disabled. */
static long lmb_add_region_flags(struct lmb_region *rgn, phys_addr_t base,
				 phys_size_t size, enum lmb_flags flags)
{
	unsigned long coalesced = 0;
	long adjacent, i;

	if (rgn->cnt == 0) {
		rgn->region[0].base = base;
		rgn->region[0].size = size;
		rgn->region[0].flags = flags;
		rgn->cnt = 1;
		return 0;
	}

	/* First try and coalesce this LMB with another. */
	for (i = 0; i < rgn->cnt; i++) {
		phys_addr_t rgnbase = rgn->region[i].base;
		phys_size_t rgnsize = rgn->region[i].size;
		phys_size_t rgnflags = rgn->region[i].flags;

		if (rgnbase == base && rgnsize == size) {
			if (flags == rgnflags)
				/* Already have this region, so we're done */
				return 0;
			else
				return -1; /* regions with new flags */
		}

		adjacent = lmb_addrs_adjacent(base, size, rgnbase, rgnsize);
		if (adjacent > 0) {
			if (flags != rgnflags)
				break;
			rgn->region[i].base -= size;
			rgn->region[i].size += size;
			coalesced++;
			break;
		} else if (adjacent < 0) {
			if (flags != rgnflags)
				break;
			rgn->region[i].size += size;
			coalesced++;
			break;
		} else if (lmb_addrs_overlap(base, size, rgnbase, rgnsize)) {
			/* regions overlap */
			return -1;
		}
	}

	if ((i < rgn->cnt - 1) && lmb_regions_adjacent(rgn, i, i + 1)) {
		if (rgn->region[i].flags == rgn->region[i + 1].flags) {
			lmb_coalesce_regions(rgn, i, i + 1);
			coalesced++;
		}
	}

	if (coalesced)
		return coalesced;
	if (rgn->cnt >= rgn->max)
		return -1;

	/* Couldn't coalesce the LMB, so add it to the sorted table. */
	for (i = rgn->cnt-1; i >= 0; i--) {
		if (base < rgn->region[i].base) {
			rgn->region[i + 1].base = rgn->region[i].base;
			rgn->region[i + 1].size = rgn->region[i].size;
			rgn->region[i + 1].flags = rgn->region[i].flags;
		} else {
			rgn->region[i + 1].base = base;
			rgn->region[i + 1].size = size;
			rgn->region[i + 1].flags = flags;
			break;
		}
	}

	if (base < rgn->region[0].base) {
		rgn->region[0].base = base;
		rgn->region[0].size = size;
		rgn->region[0].flags = flags;
	}

	rgn->cnt++;

	return 0;
}

static long lmb_add_region(struct lmb_region *rgn, phys_addr_t base,
			   phys_size_t size)
{
	return lmb_add_region_flags(rgn, base, size, LMB_NONE);
}

/* This routine may be called with relocation disabled. */
long lmb_add(struct lmb *lmb, phys_addr_t base, phys_size_t size)
{
	struct lmb_region *_rgn = &(lmb->memory);

	return lmb_add_region(_rgn, base, size);
}

long lmb_free(struct lmb *lmb, phys_addr_t base, phys_size_t size)
{
	struct lmb_region *rgn = &(lmb->reserved);
	phys_addr_t rgnbegin, rgnend;
	phys_addr_t end = base + size - 1;
	int i;

	rgnbegin = rgnend = 0; /* supress gcc warnings */

	/* Find the region where (base, size) belongs to */
	for (i = 0; i < rgn->cnt; i++) {
		rgnbegin = rgn->region[i].base;
		rgnend = rgnbegin + rgn->region[i].size - 1;

		if ((rgnbegin <= base) && (end <= rgnend))
			break;
	}

	/* Didn't find the region */
	if (i == rgn->cnt)
		return -1;

	/* Check to see if we are removing entire region */
	if ((rgnbegin == base) && (rgnend == end)) {
		lmb_remove_region(rgn, i);
		return 0;
	}

	/* Check to see if region is matching at the front */
	if (rgnbegin == base) {
		rgn->region[i].base = end + 1;
		rgn->region[i].size -= size;
		return 0;
	}

	/* Check to see if the region is matching at the end */
	if (rgnend == end) {
		rgn->region[i].size -= size;
		return 0;
	}

	/*
	 * We need to split the entry -  adjust the current one to the
	 * beginging of the hole and add the region after hole.
	 */
	rgn->region[i].size = base - rgn->region[i].base;
	return lmb_add_region_flags(rgn, end + 1, rgnend - end,
				    rgn->region[i].flags);
}

long lmb_reserve_flags(struct lmb *lmb, phys_addr_t base, phys_size_t size,
		       enum lmb_flags flags)
{
	struct lmb_region *_rgn = &(lmb->reserved);

	return lmb_add_region_flags(_rgn, base, size, flags);
}

long lmb_reserve(struct lmb *lmb, phys_addr_t base, phys_size_t size)
{
	return lmb_reserve_flags(lmb, base, size, LMB_NONE);
}

static long lmb_overlaps_region(struct lmb_region *rgn, phys_addr_t base,
				phys_size_t size)
{
	unsigned long i;

	for (i = 0; i < rgn->cnt; i++) {
		phys_addr_t rgnbase = rgn->region[i].base;
		phys_size_t rgnsize = rgn->region[i].size;
		if (lmb_addrs_overlap(base, size, rgnbase, rgnsize))
			break;
	}

	return (i < rgn->cnt) ? i : -1;
}

phys_addr_t lmb_alloc(struct lmb *lmb, phys_size_t size, ulong align)
{
	return lmb_alloc_base(lmb, size, align, LMB_ALLOC_ANYWHERE);
}

phys_addr_t lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align, phys_addr_t max_addr)
{
	phys_addr_t alloc;

	alloc = __lmb_alloc_base(lmb, size, align, max_addr);

	if (alloc == 0)
		printf("ERROR: Failed to allocate 0x%lx bytes below 0x%lx.\n",
		       (ulong)size, (ulong)max_addr);

	return alloc;
}

static phys_addr_t lmb_align_down(phys_addr_t addr, phys_size_t size)
{
	return addr & ~(size - 1);
}

phys_addr_t __lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align, phys_addr_t max_addr)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	for (i = lmb->memory.cnt - 1; i >= 0; i--) {
		phys_addr_t lmbbase = lmb->memory.region[i].base;
		phys_size_t lmbsize = lmb->memory.region[i].size;

		if (lmbsize < size)
			continue;
		if (max_addr == LMB_ALLOC_ANYWHERE)
			base = lmb_align_down(lmbbase + lmbsize - size, align);
		else if (lmbbase < max_addr) {
			base = lmbbase + lmbsize;
			if (base < lmbbase)
				base = -1;
			base = min(base, max_addr);
			base = lmb_align_down(base - size, align);
		} else
			continue;

		while (base && lmbbase <= base) {
			rgn = lmb_overlaps_region(&lmb->reserved, base, size);
			if (rgn < 0) {
				/* This area isn't reserved, take it */
				if (lmb_add_region(&lmb->reserved, base,
						   size) < 0)
					return 0;
				return base;
			}
			res_base = lmb->reserved.region[rgn].base;
			if (res_base < size)
				break;
			base = lmb_align_down(res_base - size, align);
		}
	}
	return 0;
}

/*
 * Try to allocate a specific address range: must be in defined memory but not
 * reserved
 */
phys_addr_t lmb_alloc_addr(struct lmb *lmb, phys_addr_t base, phys_size_t size)
{
	long rgn;

	/* Check if the requested address is in one of the memory regions */
	rgn = lmb_overlaps_region(&lmb->memory, base, size);
	if (rgn >= 0) {
		/*
		 * Check if the requested end address is in the same memory
		 * region we found.
		 */
		if (lmb_addrs_overlap(lmb->memory.region[rgn].base,
				      lmb->memory.region[rgn].size,
				      base + size - 1, 1)) {
			/* ok, reserve the memory */
			if (lmb_reserve(lmb, base, size) >= 0)
				return base;
		}
	}
	return 0;
}

/* Return number of bytes from a given address that are free */
phys_size_t lmb_get_free_size(struct lmb *lmb, phys_addr_t addr)
{
	int i;
	long rgn;

	/* check if the requested address is in the memory regions */
	rgn = lmb_overlaps_region(&lmb->memory, addr, 1);
	if (rgn >= 0) {
		for (i = 0; i < lmb->reserved.cnt; i++) {
			if (addr < lmb->reserved.region[i].base) {
				/* first reserved range > requested address */
				return lmb->reserved.region[i].base - addr;
			}
			if (lmb->reserved.region[i].base +
			    lmb->reserved.region[i].size > addr) {
				/* requested addr is in this reserved range */
				return 0;
			}
		}
		/* if we come here: no reserved ranges above requested addr */
		return lmb->memory.region[lmb->memory.cnt - 1].base +
		       lmb->memory.region[lmb->memory.cnt - 1].size - addr;
	}
	return 0;
}

int lmb_is_reserved_flags(struct lmb *lmb, phys_addr_t addr, int flags)
{
	int i;

	for (i = 0; i < lmb->reserved.cnt; i++) {
		phys_addr_t upper = lmb->reserved.region[i].base +
			lmb->reserved.region[i].size - 1;
		if ((addr >= lmb->reserved.region[i].base) && (addr <= upper))
			return (lmb->reserved.region[i].flags & flags) == flags;
	}
	return 0;
}

int lmb_is_reserved(struct lmb *lmb, phys_addr_t addr)
{
	return lmb_is_reserved_flags(lmb, addr, LMB_NONE);
}

__weak void board_lmb_reserve(struct lmb *lmb)
{
	/* please define platform specific board_lmb_reserve() */
}

__weak void arch_lmb_reserve(struct lmb *lmb)
{
	/* please define platform specific arch_lmb_reserve() */
}
