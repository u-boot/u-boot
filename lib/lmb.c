// SPDX-License-Identifier: GPL-2.0+
/*
 * Procedures for maintaining information about logical memory blocks.
 *
 * Peter Bergner, IBM Corp.	June 2001.
 * Copyright (C) 2001 Peter Bergner.
 */

#include <efi_loader.h>
#include <event.h>
#include <image.h>
#include <mapmem.h>
#include <lmb.h>
#include <log.h>
#include <malloc.h>

#include <asm/global_data.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAP_OP_RESERVE		(u8)0x1
#define MAP_OP_FREE		(u8)0x2

#define LMB_ALLOC_ANYWHERE	0

extern bool is_addr_in_ram(uintptr_t addr);

#if !IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
struct lmb_property memory_regions[CONFIG_LMB_MEMORY_REGIONS];
struct lmb_property reserved_regions[CONFIG_LMB_RESERVED_REGIONS];
#endif

struct lmb lmb = {
#if IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
	.memory.max = CONFIG_LMB_MAX_REGIONS,
	.reserved.max = CONFIG_LMB_MAX_REGIONS,
#else
	.memory.max = CONFIG_LMB_MEMORY_REGIONS,
	.reserved.max = CONFIG_LMB_RESERVED_REGIONS,
	.memory.region = memory_regions,
	.reserved.region = reserved_regions,
#endif
	.memory.cnt = 0,
	.reserved.cnt = 0,
};

static void lmb_map_update_notify(phys_addr_t addr, phys_size_t size,
				  u8 op)
{
	struct event_lmb_map_update lmb_map = {0};

	lmb_map.base = addr;
	lmb_map.size = size;
	lmb_map.op = op;

	if (is_addr_in_ram((uintptr_t)addr))
		event_notify(EVT_LMB_MAP_UPDATE, &lmb_map, sizeof(lmb_map));
}

static void lmb_dump_region(struct lmb_region *rgn, char *name)
{
	unsigned long long base, size, end;
	enum lmb_flags flags;
	int i;

	printf(" %s.cnt = 0x%lx / max = 0x%lx\n", name, rgn->cnt, rgn->max);

	for (i = 0; i < rgn->cnt; i++) {
		base = rgn->region[i].base;
		size = rgn->region[i].size;
		end = base + size - 1;
		flags = rgn->region[i].flags;

		printf(" %s[%d]\t[0x%llx-0x%llx], 0x%08llx bytes flags: %x\n",
		       name, i, base, end, size, flags);
	}
}

void lmb_dump_all_force(void)
{
	printf("lmb_dump_all:\n");
	lmb_dump_region(&lmb.memory, "memory");
	lmb_dump_region(&lmb.reserved, "reserved");
}

void lmb_dump_all(void)
{
#ifdef DEBUG
	lmb_dump_all_force();
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

static long lmb_regions_overlap(struct lmb_region *rgn, unsigned long r1,
				unsigned long r2)
{
	phys_addr_t base1 = rgn->region[r1].base;
	phys_size_t size1 = rgn->region[r1].size;
	phys_addr_t base2 = rgn->region[r2].base;
	phys_size_t size2 = rgn->region[r2].size;

	return lmb_addrs_overlap(base1, size1, base2, size2);
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

/*Assumption : base addr of region 1 < base addr of region 2*/
static void lmb_fix_over_lap_regions(struct lmb_region *rgn, unsigned long r1,
				     unsigned long r2)
{
	phys_addr_t base1 = rgn->region[r1].base;
	phys_size_t size1 = rgn->region[r1].size;
	phys_addr_t base2 = rgn->region[r2].base;
	phys_size_t size2 = rgn->region[r2].size;

	if (base1 + size1 > base2 + size2) {
		printf("This will not be a case any time\n");
		return;
	}
	rgn->region[r1].size = base2 + size2 - base1;
	lmb_remove_region(rgn, r2);
}

void arch_lmb_reserve_generic(ulong sp, ulong end, ulong align)
{
	ulong bank_end;
	int bank;

	/*
	 * Reserve memory from aligned address below the bottom of U-Boot stack
	 * until end of U-Boot area using LMB to prevent U-Boot from overwriting
	 * that memory.
	 */
	debug("## Current stack ends at 0x%08lx ", sp);

	/* adjust sp by 4K to be safe */
	sp -= align;
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (!gd->bd->bi_dram[bank].size ||
		    sp < gd->bd->bi_dram[bank].start)
			continue;
		/* Watch out for RAM at end of address space! */
		bank_end = gd->bd->bi_dram[bank].start +
			gd->bd->bi_dram[bank].size - 1;
		if (sp > bank_end)
			continue;
		if (bank_end > end)
			bank_end = end - 1;

		lmb_reserve(sp, bank_end - sp + 1);

		if (gd->flags & GD_FLG_SKIP_RELOC)
			lmb_reserve((phys_addr_t)(uintptr_t)_start, gd->mon_len);

		break;
	}
}

/**
 * efi_lmb_reserve() - add reservations for EFI memory
 *
 * Add reservations for all EFI memory areas that are not
 * EFI_CONVENTIONAL_MEMORY.
 *
 * Return:	0 on success, 1 on failure
 */
static __maybe_unused int efi_lmb_reserve(void)
{
	struct efi_mem_desc *memmap = NULL, *map;
	efi_uintn_t i, map_size = 0;
	efi_status_t ret;

	ret = efi_get_memory_map_alloc(&map_size, &memmap);
	if (ret != EFI_SUCCESS)
		return 1;

	for (i = 0, map = memmap; i < map_size / sizeof(*map); ++map, ++i) {
		if (map->type != EFI_CONVENTIONAL_MEMORY) {
			lmb_reserve_flags(map_to_sysmem((void *)(uintptr_t)
							map->physical_start),
					  map->num_pages * EFI_PAGE_SIZE,
					  map->type == EFI_RESERVED_MEMORY_TYPE
					      ? LMB_NOMAP : LMB_NONE);
		}
	}
	efi_free_pool(memmap);

	return 0;
}

/**
 * lmb_reserve_common() - Reserve memory region occupied by U-Boot image
 * @fdt_blob: pointer to the FDT blob
 *
 * Reserve common areas of memory that are occupied by the U-Boot image.
 * This function gets called once U-Boot has been relocated, so that any
 * request for memory allocations would not touch memory region occupied
 * by the U-Boot image, heap, bss etc.
 *
 * Return: None
 *
 */
void lmb_reserve_common(void *fdt_blob)
{
	arch_lmb_reserve();
	board_lmb_reserve();

	if (CONFIG_IS_ENABLED(OF_LIBFDT) && fdt_blob)
		boot_fdt_add_mem_rsv_regions(fdt_blob);

	if (CONFIG_IS_ENABLED(EFI_LOADER))
		efi_lmb_reserve();
}

/**
 * lmb_add_memory() - Add memory range for LMB allocations
 * @bd: pointer to board info structure
 *
 * Add the entire available memory range to the pool of memory that
 * can be used by the LMB module for allocations.
 *
 * Return: None
 *
 */
void lmb_add_memory(struct bd_info *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (bd->bi_dram[i].size)
			lmb_add(bd->bi_dram[i].start, bd->bi_dram[i].size);
	}
}

static bool lmb_region_flags_match(struct lmb_region *rgn, unsigned long r1,
				   enum lmb_flags flags)
{
	return rgn->region[r1].flags == flags;
}

static long lmb_merge_overlap_regions(struct lmb_region *rgn, unsigned long i,
				      phys_addr_t base, phys_size_t size,
				      enum lmb_flags flags)
{
	phys_size_t rgnsize;
	unsigned long rgn_cnt, idx;
	phys_addr_t rgnbase, rgnend;
	phys_addr_t mergebase, mergeend;

	rgn_cnt = 0;
	idx = i;
	/*
	 * First thing to do is to identify how many regions does
	 * the requested region overlap.
	 * If the flags match, combine all these overlapping
	 * regions into a single region, and remove the merged
	 * regions.
	 */
	while (idx < rgn->cnt - 1) {
		rgnbase = rgn->region[idx].base;
		rgnsize = rgn->region[idx].size;

		if (lmb_addrs_overlap(base, size, rgnbase,
				      rgnsize)) {
			if (!lmb_region_flags_match(rgn, idx, flags))
				return -1;
			rgn_cnt++;
			idx++;
		}
	}

	/* The merged region's base and size */
	rgnbase = rgn->region[i].base;
	mergebase = min(base, rgnbase);
	rgnend = rgn->region[idx].base + rgn->region[idx].size;
	mergeend = max(rgnend, (base + size));

	rgn->region[i].base = mergebase;
	rgn->region[i].size = mergeend - mergebase;

	/* Now remove the merged regions */
	while (--rgn_cnt)
		lmb_remove_region(rgn, i + 1);

	return 0;
}

static long lmb_resize_regions(struct lmb_region *rgn, unsigned long i,
			       phys_addr_t base, phys_size_t size,
			       enum lmb_flags flags)
{
	long ret = 0;
	phys_addr_t rgnend;

	if (i == rgn->cnt - 1 ||
		base + size < rgn->region[i + 1].base) {
		if (!lmb_region_flags_match(rgn, i, flags))
			return -1;

		rgnend = rgn->region[i].base + rgn->region[i].size;
		rgn->region[i].base = min(base, rgn->region[i].base);
		rgnend = max(base + size, rgnend);
		rgn->region[i].size = rgnend - rgn->region[i].base;
	} else {
		ret = lmb_merge_overlap_regions(rgn, i, base, size, flags);
	}

	return ret;
}

/* This routine called with relocation disabled. */
static long lmb_add_region_flags(struct lmb_region *rgn, phys_addr_t base,
				 phys_size_t size, enum lmb_flags flags)
{
	unsigned long coalesced = 0;
	long ret, i;

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
		phys_addr_t end = base + size - 1;
		phys_addr_t rgnend = rgnbase + rgnsize - 1;
		if (rgnbase <= base && end <= rgnend) {
			if (flags == rgnflags)
				/* Already have this region, so we're done */
				return 0;
			else
				return -1; /* regions with new flags */
		}

		ret = lmb_addrs_adjacent(base, size, rgnbase, rgnsize);
		if (ret > 0) {
			if (flags != rgnflags)
				break;
			rgn->region[i].base -= size;
			rgn->region[i].size += size;
			coalesced++;
			break;
		} else if (ret < 0) {
			if (flags != rgnflags)
				break;
			rgn->region[i].size += size;
			coalesced++;
			break;
		} else if (lmb_addrs_overlap(base, size, rgnbase, rgnsize)) {
			if (flags == LMB_NONE) {
				ret = lmb_resize_regions(rgn, i, base, size,
							 flags);
				if (ret < 0)
					return -1;

				coalesced++;
				break;
			} else {
				return -1;
			}
		}
	}

	if (i < rgn->cnt - 1 && rgn->region[i].flags == rgn->region[i + 1].flags)  {
		if (lmb_regions_adjacent(rgn, i, i + 1)) {
			lmb_coalesce_regions(rgn, i, i + 1);
			coalesced++;
		} else if (lmb_regions_overlap(rgn, i, i + 1)) {
			/* fix overlapping area */
			lmb_fix_over_lap_regions(rgn, i, i + 1);
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
long lmb_add(phys_addr_t base, phys_size_t size)
{
	struct lmb_region *_rgn = &lmb.memory;

	return lmb_add_region(_rgn, base, size);
}

static long __lmb_free(phys_addr_t base, phys_size_t size)
{
	struct lmb_region *rgn = &lmb.reserved;
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

long lmb_free(phys_addr_t base, phys_size_t size)
{
	long ret;

	ret = __lmb_free(base, size);
	if (ret < 0)
		return ret;

	if (CONFIG_IS_ENABLED(MEM_MAP_UPDATE_NOTIFY))
		lmb_map_update_notify(base, size, MAP_OP_FREE);

	return 0;
}

long lmb_reserve_flags(phys_addr_t base, phys_size_t size, enum lmb_flags flags)
{
	long ret = 0;
	struct lmb_region *_rgn = &lmb.reserved;

	ret = lmb_add_region_flags(_rgn, base, size, flags);
	if (ret < 0)
		return -1;

	if (CONFIG_IS_ENABLED(MEM_MAP_UPDATE_NOTIFY))
		lmb_map_update_notify(base, size, MAP_OP_RESERVE);

	return ret;
}

long lmb_reserve(phys_addr_t base, phys_size_t size)
{
	return lmb_reserve_flags(base, size, LMB_NONE);
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

static phys_addr_t lmb_align_down(phys_addr_t addr, phys_size_t size)
{
	return addr & ~(size - 1);
}

static phys_addr_t __lmb_alloc_base(phys_size_t size, ulong align,
				    phys_addr_t max_addr, enum lmb_flags flags)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	for (i = lmb.memory.cnt - 1; i >= 0; i--) {
		phys_addr_t lmbbase = lmb.memory.region[i].base;
		phys_size_t lmbsize = lmb.memory.region[i].size;

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
			rgn = lmb_overlaps_region(&lmb.reserved, base, size);
			if (rgn < 0) {
				/* This area isn't reserved, take it */
				if (lmb_add_region(&lmb.reserved, base,
						   size) < 0)
					return 0;

				if (CONFIG_IS_ENABLED(MEM_MAP_UPDATE_NOTIFY))
					lmb_map_update_notify(base, size,
							      MAP_OP_RESERVE);
				return base;
			}
			res_base = lmb.reserved.region[rgn].base;
			if (res_base < size)
				break;
			base = lmb_align_down(res_base - size, align);
		}
	}
	return 0;
}

phys_addr_t lmb_alloc(phys_size_t size, ulong align)
{
	return lmb_alloc_base(size, align, LMB_ALLOC_ANYWHERE);
}

phys_addr_t lmb_alloc_base(phys_size_t size, ulong align, phys_addr_t max_addr)
{
	phys_addr_t alloc;

	alloc = __lmb_alloc_base(size, align, max_addr, LMB_NONE);

	if (alloc == 0)
		printf("ERROR: Failed to allocate 0x%lx bytes below 0x%lx.\n",
		       (ulong)size, (ulong)max_addr);

	return alloc;
}

static phys_addr_t __lmb_alloc_addr(phys_addr_t base, phys_size_t size,
				    enum lmb_flags flags)
{
	long rgn;

	/* Check if the requested address is in one of the memory regions */
	rgn = lmb_overlaps_region(&lmb.memory, base, size);
	if (rgn >= 0) {
		/*
		 * Check if the requested end address is in the same memory
		 * region we found.
		 */
		if (lmb_addrs_overlap(lmb.memory.region[rgn].base,
				      lmb.memory.region[rgn].size,
				      base + size - 1, 1)) {
			/* ok, reserve the memory */
			if (lmb_reserve_flags(base, size, flags) >= 0)
				return base;
		}
	}

	return 0;
}

/*
 * Try to allocate a specific address range: must be in defined memory but not
 * reserved
 */
phys_addr_t lmb_alloc_addr(phys_addr_t base, phys_size_t size)
{
	return __lmb_alloc_addr(base, size, LMB_NONE);
}

/* Return number of bytes from a given address that are free */
phys_size_t lmb_get_free_size(phys_addr_t addr)
{
	int i;
	long rgn;

	/* check if the requested address is in the memory regions */
	rgn = lmb_overlaps_region(&lmb.memory, addr, 1);
	if (rgn >= 0) {
		for (i = 0; i < lmb.reserved.cnt; i++) {
			if (addr < lmb.reserved.region[i].base) {
				/* first reserved range > requested address */
				return lmb.reserved.region[i].base - addr;
			}
			if (lmb.reserved.region[i].base +
			    lmb.reserved.region[i].size > addr) {
				/* requested addr is in this reserved range */
				return 0;
			}
		}
		/* if we come here: no reserved ranges above requested addr */
		return lmb.memory.region[lmb.memory.cnt - 1].base +
		       lmb.memory.region[lmb.memory.cnt - 1].size - addr;
	}
	return 0;
}

int lmb_is_reserved_flags(phys_addr_t addr, int flags)
{
	int i;

	for (i = 0; i < lmb.reserved.cnt; i++) {
		phys_addr_t upper = lmb.reserved.region[i].base +
			lmb.reserved.region[i].size - 1;
		if ((addr >= lmb.reserved.region[i].base) && (addr <= upper))
			return (lmb.reserved.region[i].flags & flags) == flags;
	}
	return 0;
}

__weak void board_lmb_reserve(void)
{
	/* please define platform specific board_lmb_reserve() */
}

__weak void arch_lmb_reserve(void)
{
	/* please define platform specific arch_lmb_reserve() */
}
