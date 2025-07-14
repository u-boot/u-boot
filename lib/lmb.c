// SPDX-License-Identifier: GPL-2.0+
/*
 * Procedures for maintaining information about logical memory blocks.
 *
 * Peter Bergner, IBM Corp.	June 2001.
 * Copyright (C) 2001 Peter Bergner.
 */

#include <alist.h>
#include <efi_loader.h>
#include <env.h>
#include <event.h>
#include <image.h>
#include <mapmem.h>
#include <lmb.h>
#include <log.h>
#include <malloc.h>
#include <spl.h>

#include <asm/global_data.h>
#include <asm/sections.h>
#include <linux/kernel.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define LMB_RGN_OVERLAP		1
#define LMB_RGN_ADJACENT	2

/*
 * The following low level LMB functions must not access the global LMB memory
 * map since they are also used to manage IOVA memory maps in iommu drivers like
 * apple_dart.
 */

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

/**
 * lmb_regions_check() - Check if the regions overlap, or are adjacent
 * @lmb_rgn_lst: List of LMB regions
 * @r1: First region to check
 * @r2: Second region to check
 *
 * Check if the two regions with matching flags, r1 and r2 are
 * adjacent to each other, or if they overlap.
 *
 * Return:
 * * %LMB_RGN_OVERLAP	- Regions overlap
 * * %LMB_RGN_ADJACENT	- Regions adjacent to each other
 * * 0			- Neither of the above, or flags mismatch
 */
static long lmb_regions_check(struct alist *lmb_rgn_lst, unsigned long r1,
			      unsigned long r2)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;
	phys_addr_t base1 = rgn[r1].base;
	phys_size_t size1 = rgn[r1].size;
	phys_addr_t base2 = rgn[r2].base;
	phys_size_t size2 = rgn[r2].size;

	if (rgn[r1].flags != rgn[r2].flags)
		return 0;

	if (lmb_addrs_overlap(base1, size1, base2, size2))
		return LMB_RGN_OVERLAP;
	else if (lmb_addrs_adjacent(base1, size1, base2, size2))
		return LMB_RGN_ADJACENT;

	return 0;
}

static void lmb_remove_region(struct alist *lmb_rgn_lst, unsigned long r)
{
	unsigned long i;
	struct lmb_region *rgn = lmb_rgn_lst->data;

	for (i = r; i < lmb_rgn_lst->count - 1; i++) {
		rgn[i].base = rgn[i + 1].base;
		rgn[i].size = rgn[i + 1].size;
		rgn[i].flags = rgn[i + 1].flags;
	}
	lmb_rgn_lst->count--;
}

/* Assumption: base addr of region 1 < base addr of region 2 */
static void lmb_coalesce_regions(struct alist *lmb_rgn_lst, unsigned long r1,
				 unsigned long r2)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;

	rgn[r1].size += rgn[r2].size;
	lmb_remove_region(lmb_rgn_lst, r2);
}

static long lmb_resize_regions(struct alist *lmb_rgn_lst,
			       unsigned long idx_start,
			       phys_addr_t base, phys_size_t size)
{
	phys_size_t rgnsize;
	unsigned long rgn_cnt, idx, idx_end;
	phys_addr_t rgnbase, rgnend;
	phys_addr_t mergebase, mergeend;
	struct lmb_region *rgn = lmb_rgn_lst->data;

	rgn_cnt = 0;
	idx = idx_start;
	idx_end = idx_start;

	/*
	 * First thing to do is to identify how many regions
	 * the requested region overlaps.
	 * If the flags match, combine all these overlapping
	 * regions into a single region, and remove the merged
	 * regions.
	 */
	while (idx <= lmb_rgn_lst->count - 1) {
		rgnbase = rgn[idx].base;
		rgnsize = rgn[idx].size;

		if (lmb_addrs_overlap(base, size, rgnbase,
				      rgnsize)) {
			if (rgn[idx].flags != LMB_NONE)
				return -1;
			rgn_cnt++;
			idx_end = idx;
		}
		idx++;
	}

	/* The merged region's base and size */
	rgnbase = rgn[idx_start].base;
	mergebase = min(base, rgnbase);
	rgnend = rgn[idx_end].base + rgn[idx_end].size;
	mergeend = max(rgnend, (base + size));

	rgn[idx_start].base = mergebase;
	rgn[idx_start].size = mergeend - mergebase;

	/* Now remove the merged regions */
	while (--rgn_cnt)
		lmb_remove_region(lmb_rgn_lst, idx_start + 1);

	return 0;
}

/**
 * lmb_add_region_flags() - Add an lmb region to the given list
 * @lmb_rgn_lst: LMB list to which region is to be added(free/used)
 * @base: Start address of the region
 * @size: Size of the region to be added
 * @flags: Attributes of the LMB region
 *
 * Add a region of memory to the list. If the region does not exist, add
 * it to the list. Depending on the attributes of the region to be added,
 * the function might resize an already existing region or coalesce two
 * adjacent regions.
 *
 * Return:
 * * %0		- Added successfully, or it's already added (only if LMB_NONE)
 * * %-EEXIST	- The region is already added, and flags != LMB_NONE
 * * %-1	- Failure
 */
static long lmb_add_region_flags(struct alist *lmb_rgn_lst, phys_addr_t base,
				 phys_size_t size, u32 flags)
{
	unsigned long coalesced = 0;
	long ret, i;
	struct lmb_region *rgn = lmb_rgn_lst->data;

	if (alist_err(lmb_rgn_lst))
		return -1;

	/* First try and coalesce this LMB with another. */
	for (i = 0; i < lmb_rgn_lst->count; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;
		u32 rgnflags = rgn[i].flags;

		ret = lmb_addrs_adjacent(base, size, rgnbase, rgnsize);
		if (ret > 0) {
			if (flags != rgnflags)
				break;
			rgn[i].base -= size;
			rgn[i].size += size;
			coalesced++;
			break;
		} else if (ret < 0) {
			if (flags != rgnflags)
				continue;
			rgn[i].size += size;
			coalesced++;
			break;
		} else if (lmb_addrs_overlap(base, size, rgnbase, rgnsize)) {
			ret = lmb_resize_regions(lmb_rgn_lst, i, base, size);
			if (ret < 0)
				return -1;

			coalesced++;
			break;
		}
	}

	if (lmb_rgn_lst->count && i < lmb_rgn_lst->count - 1) {
		ret = lmb_regions_check(lmb_rgn_lst, i, i + 1);
		if (ret == LMB_RGN_ADJACENT) {
			lmb_coalesce_regions(lmb_rgn_lst, i, i + 1);
			coalesced++;
		} else if (ret == LMB_RGN_OVERLAP) {
			/* fix overlapping areas */
			phys_addr_t rgnbase = rgn[i].base;
			phys_size_t rgnsize = rgn[i].size;

			ret = lmb_resize_regions(lmb_rgn_lst, i,
						 rgnbase, rgnsize);
			if (ret < 0)
				return -1;

			coalesced++;
		}
	}

	if (coalesced)
		return 0;

	if (alist_full(lmb_rgn_lst) &&
	    !alist_expand_by(lmb_rgn_lst, lmb_rgn_lst->alloc))
		return -1;
	rgn = lmb_rgn_lst->data;

	/* Couldn't coalesce the LMB, so add it to the sorted table. */
	for (i = lmb_rgn_lst->count; i >= 0; i--) {
		if (i && base < rgn[i - 1].base) {
			rgn[i] = rgn[i - 1];
		} else {
			rgn[i].base = base;
			rgn[i].size = size;
			rgn[i].flags = flags;
			break;
		}
	}

	lmb_rgn_lst->count++;

	return 0;
}

static long _lmb_free(struct alist *lmb_rgn_lst, phys_addr_t base,
		      phys_size_t size)
{
	struct lmb_region *rgn;
	phys_addr_t rgnbegin, rgnend;
	phys_addr_t end = base + size - 1;
	int i;

	/* Suppress GCC warnings */
	rgnbegin = 0;
	rgnend = 0;

	rgn = lmb_rgn_lst->data;
	/* Find the region where (base, size) belongs to */
	for (i = 0; i < lmb_rgn_lst->count; i++) {
		rgnbegin = rgn[i].base;
		rgnend = rgnbegin + rgn[i].size - 1;

		if (rgnbegin <= base && end <= rgnend)
			break;
	}

	/* Didn't find the region */
	if (i == lmb_rgn_lst->count)
		return -1;

	/* Check to see if we are removing entire region */
	if (rgnbegin == base && rgnend == end) {
		lmb_remove_region(lmb_rgn_lst, i);
		return 0;
	}

	/* Check to see if region is matching at the front */
	if (rgnbegin == base) {
		rgn[i].base = end + 1;
		rgn[i].size -= size;
		return 0;
	}

	/* Check to see if the region is matching at the end */
	if (rgnend == end) {
		rgn[i].size -= size;
		return 0;
	}

	/*
	 * We need to split the entry -  adjust the current one to the
	 * beginging of the hole and add the region after hole.
	 */
	rgn[i].size = base - rgn[i].base;
	return lmb_add_region_flags(lmb_rgn_lst, end + 1, rgnend - end,
				    rgn[i].flags);
}

/**
 * lmb_overlap_checks() - perform checks to see if region can be allocated or reserved
 * @lmb_rgn_lst: list of LMB regions
 * @base: base address of region to be checked
 * @size: size of region to be checked
 * @flags: flag of the region to be checked (only for reservation requests)
 * @alloc: if checks are to be done for allocation or reservation request
 *
 * Check if the region passed to the function overlaps with any one of
 * the regions of the passed lmb region list.
 *
 * If the @alloc flag is set to true, this check stops as soon an
 * overlapping region is found. The function can also be called to
 * check if a reservation request can be satisfied, by setting
 * @alloc to false. In that case, the function then iterates through
 * all the regions in the list passed to ensure that the requested
 * region does not overlap with any existing regions. An overlap is
 * allowed only when the flag of the requested region and the existing
 * region is LMB_NONE.
 *
 * Return: index of the overlapping region, -1 if no overlap is found
 *
 * When the function is called for a reservation request check, -1 will
 * also be returned when there is an allowed overlap, i.e. requested
 * region and existing regions have flags as LMB_NONE.
 */
static long lmb_overlap_checks(struct alist *lmb_rgn_lst, phys_addr_t base,
			       phys_size_t size, u32 flags, bool alloc)
{
	unsigned long i;
	struct lmb_region *rgn = lmb_rgn_lst->data;

	for (i = 0; i < lmb_rgn_lst->count; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;
		u32 rgnflags = rgn[i].flags;

		if (lmb_addrs_overlap(base, size, rgnbase, rgnsize)) {
			if (alloc || flags != LMB_NONE || flags != rgnflags)
				break;
		}
	}

	return (i < lmb_rgn_lst->count) ? i : -1;
}

/*
 * IOVA LMB memory maps using lmb pointers instead of the global LMB memory map.
 */

int io_lmb_setup(struct lmb *io_lmb)
{
	int ret;

	ret = alist_init(&io_lmb->available_mem, sizeof(struct lmb_region),
			 (uint)LMB_ALIST_INITIAL_SIZE);
	if (!ret) {
		log_debug("Unable to initialise the list for LMB free IOVA\n");
		return -ENOMEM;
	}

	ret = alist_init(&io_lmb->used_mem, sizeof(struct lmb_region),
			 (uint)LMB_ALIST_INITIAL_SIZE);
	if (!ret) {
		log_debug("Unable to initialise the list for LMB used IOVA\n");
		return -ENOMEM;
	}

	io_lmb->test = false;

	return 0;
}

void io_lmb_teardown(struct lmb *io_lmb)
{
	alist_uninit(&io_lmb->available_mem);
	alist_uninit(&io_lmb->used_mem);
}

long io_lmb_add(struct lmb *io_lmb, phys_addr_t base, phys_size_t size)
{
	return lmb_add_region_flags(&io_lmb->available_mem, base, size, LMB_NONE);
}

/* derived and simplified from _lmb_alloc_base() */
phys_addr_t io_lmb_alloc(struct lmb *io_lmb, phys_size_t size, ulong align)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;
	struct lmb_region *lmb_used = io_lmb->used_mem.data;
	struct lmb_region *lmb_memory = io_lmb->available_mem.data;

	for (i = io_lmb->available_mem.count - 1; i >= 0; i--) {
		phys_addr_t lmbbase = lmb_memory[i].base;
		phys_size_t lmbsize = lmb_memory[i].size;

		if (lmbsize < size)
			continue;
		base = ALIGN_DOWN(lmbbase + lmbsize - size, align);

		while (base && lmbbase <= base) {
			rgn = lmb_overlap_checks(&io_lmb->used_mem, base, size,
						 LMB_NOOVERWRITE, true);
			if (rgn < 0) {
				/* This area isn't reserved, take it */
				if (lmb_add_region_flags(&io_lmb->used_mem, base,
							 size, LMB_NONE) < 0)
					return 0;

				return base;
			}

			res_base = lmb_used[rgn].base;
			if (res_base < size)
				break;
			base = ALIGN_DOWN(res_base - size, align);
		}
	}
	return 0;
}

long io_lmb_free(struct lmb *io_lmb, phys_addr_t base, phys_size_t size)
{
	return _lmb_free(&io_lmb->used_mem, base, size);
}

/*
 * Low level LMB functions are used to manage IOVA memory maps for the Apple
 * dart iommu. They must not access the global LMB memory map.
 * So keep the global LMB variable declaration unreachable from them.
 */

static struct lmb lmb;

static int lmb_map_update_notify(phys_addr_t addr, phys_size_t size,
				 enum lmb_map_op op, u32 flags)
{
	if (CONFIG_IS_ENABLED(EFI_LOADER) &&
	    !lmb.test && !(flags & LMB_NONOTIFY))
		return efi_map_update_notify(addr, size, op);

	return 0;
}

static void lmb_print_region_flags(u32 flags)
{
	const char * const flag_str[] = { "none", "no-map", "no-overwrite",
					  "no-notify" };
	unsigned int pflags = flags &
			      (LMB_NOMAP | LMB_NOOVERWRITE | LMB_NONOTIFY);

	if (flags != pflags) {
		printf("invalid %#x\n", flags);
		return;
	}

	do {
		int bitpos = pflags ? fls(pflags) - 1 : 0;

		printf("%s", flag_str[bitpos]);
		pflags &= ~(1u << bitpos);
		puts(pflags ? ", " : "\n");
	} while (pflags);
}

static void lmb_dump_region(struct alist *lmb_rgn_lst, char *name)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;
	unsigned long long base, size, end;
	u32 flags;
	int i;

	printf(" %s.count = %#x\n", name, lmb_rgn_lst->count);

	for (i = 0; i < lmb_rgn_lst->count; i++) {
		base = rgn[i].base;
		size = rgn[i].size;
		end = base + size - 1;
		flags = rgn[i].flags;

		printf(" %s[%d]\t[%#llx-%#llx], %#llx bytes, flags: ",
		       name, i, base, end, size);
		lmb_print_region_flags(flags);
	}
}

void lmb_dump_all_force(void)
{
	printf("lmb_dump_all:\n");
	lmb_dump_region(&lmb.available_mem, "memory");
	lmb_dump_region(&lmb.used_mem, "reserved");
}

void lmb_dump_all(void)
{
#ifdef DEBUG
	lmb_dump_all_force();
#endif
}

static long lmb_reserve(phys_addr_t base, phys_size_t size, u32 flags)
{
	long ret = 0;
	struct alist *lmb_rgn_lst = &lmb.used_mem;

	if (lmb_overlap_checks(lmb_rgn_lst, base, size, flags, false) != -1)
		return -EEXIST;

	ret = lmb_add_region_flags(lmb_rgn_lst, base, size, flags);
	if (ret)
		return ret;

	return lmb_map_update_notify(base, size, LMB_MAP_OP_RESERVE, flags);
}

static void lmb_reserve_uboot_region(void)
{
	int bank;
	ulong end, bank_end;
	phys_addr_t rsv_start;
	ulong pram = 0;

	rsv_start = gd->start_addr_sp - CONFIG_STACK_SIZE;
	end = gd->ram_top;

	/*
	 * Reserve memory from aligned address below the bottom of U-Boot stack
	 * until end of RAM area to prevent LMB from overwriting that memory.
	 */
	debug("## Current stack ends at 0x%08lx ", (ulong)rsv_start);

#ifdef CFG_PRAM
	pram = env_get_ulong("pram", 10, CFG_PRAM);
	pram = pram << 10; /* size is in kB */
#endif

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (!gd->bd->bi_dram[bank].size ||
		    rsv_start < gd->bd->bi_dram[bank].start)
			continue;
		/* Watch out for RAM at end of address space! */
		bank_end = gd->bd->bi_dram[bank].start +
			gd->bd->bi_dram[bank].size - 1;
		if (rsv_start > bank_end)
			continue;
		if (bank_end > end)
			bank_end = end - 1;

		lmb_reserve(rsv_start, bank_end - rsv_start - pram + 1,
			    LMB_NOOVERWRITE);

		if (gd->flags & GD_FLG_SKIP_RELOC)
			lmb_reserve((phys_addr_t)(uintptr_t)_start,
				    gd->mon_len, LMB_NOOVERWRITE);

		break;
	}
}

static void lmb_reserve_common(void *fdt_blob)
{
	lmb_reserve_uboot_region();

	if (CONFIG_IS_ENABLED(OF_LIBFDT) && fdt_blob)
		boot_fdt_add_mem_rsv_regions(fdt_blob);
}

static __maybe_unused void lmb_reserve_common_spl(void)
{
	phys_addr_t rsv_start;
	phys_size_t rsv_size;

	/*
	 * Assume a SPL stack of 16KB. This must be
	 * more than enough for the SPL stage.
	 */
	if (IS_ENABLED(CONFIG_SPL_STACK_R_ADDR)) {
		rsv_start = gd->start_addr_sp - 16384;
		rsv_size = 16384;
		lmb_reserve(rsv_start, rsv_size, LMB_NOOVERWRITE);
	}

	if (IS_ENABLED(CONFIG_SPL_SEPARATE_BSS)) {
		/* Reserve the bss region */
		rsv_start = (phys_addr_t)(uintptr_t)__bss_start;
		rsv_size = (phys_addr_t)(uintptr_t)__bss_end -
			(phys_addr_t)(uintptr_t)__bss_start;
		lmb_reserve(rsv_start, rsv_size, LMB_NOOVERWRITE);
	}
}

static void lmb_add_memory(void)
{
	int i;
	phys_addr_t bank_end;
	phys_size_t size;
	u64 ram_top = gd->ram_top;
	struct bd_info *bd = gd->bd;

	if (CONFIG_IS_ENABLED(LMB_ARCH_MEM_MAP))
		return lmb_arch_add_memory();

	/* Assume a 4GB ram_top if not defined */
	if (!ram_top)
		ram_top = 0x100000000ULL;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size = bd->bi_dram[i].size;
		bank_end = bd->bi_dram[i].start + size;

		if (size) {
			lmb_add(bd->bi_dram[i].start, size);

			/*
			 * Reserve memory above ram_top as
			 * no-overwrite so that it cannot be
			 * allocated
			 */
			if (bd->bi_dram[i].start >= ram_top)
				lmb_reserve(bd->bi_dram[i].start, size,
					    LMB_NOOVERWRITE);
			else if (bank_end > ram_top)
				lmb_reserve(ram_top, bank_end - ram_top,
					    LMB_NOOVERWRITE);
		}
	}
}

/* This routine may be called with relocation disabled. */
long lmb_add(phys_addr_t base, phys_size_t size)
{
	long ret;
	struct alist *lmb_rgn_lst = &lmb.available_mem;

	ret = lmb_add_region_flags(lmb_rgn_lst, base, size, LMB_NONE);
	if (ret)
		return ret;

	return lmb_map_update_notify(base, size, LMB_MAP_OP_ADD, LMB_NONE);
}

long lmb_free(phys_addr_t base, phys_size_t size, u32 flags)
{
	long ret;

	ret = _lmb_free(&lmb.used_mem, base, size);
	if (ret < 0)
		return ret;

	return lmb_map_update_notify(base, size, LMB_MAP_OP_FREE, flags);
}

static int _lmb_alloc_base(phys_size_t size, ulong align,
			   phys_addr_t *addr, u32 flags)
{
	int ret;
	long i, rgn;
	phys_addr_t max_addr;
	phys_addr_t base = 0;
	phys_addr_t res_base;
	struct lmb_region *lmb_used = lmb.used_mem.data;
	struct lmb_region *lmb_memory = lmb.available_mem.data;

	max_addr = *addr;
	for (i = lmb.available_mem.count - 1; i >= 0; i--) {
		phys_addr_t lmbbase = lmb_memory[i].base;
		phys_size_t lmbsize = lmb_memory[i].size;

		if (lmbsize < size)
			continue;

		if (max_addr == LMB_ALLOC_ANYWHERE) {
			base = ALIGN_DOWN(lmbbase + lmbsize - size, align);
		} else if (lmbbase < max_addr) {
			base = lmbbase + lmbsize;
			if (base < lmbbase)
				base = -1;
			base = min(base, max_addr);
			base = ALIGN_DOWN(base - size, align);
		} else {
			continue;
		}

		while (base && lmbbase <= base) {
			rgn = lmb_overlap_checks(&lmb.used_mem, base, size,
						 LMB_NOOVERWRITE, true);
			if (rgn < 0) {
				/* This area isn't reserved, take it */
				if (lmb_add_region_flags(&lmb.used_mem, base,
							 size, flags))
					return 0;

				ret = lmb_map_update_notify(base, size,
							    LMB_MAP_OP_RESERVE,
							    flags);
				if (ret)
					return ret;
				*addr = base;
				return 0;
			}

			res_base = lmb_used[rgn].base;
			if (res_base < size)
				break;
			base = ALIGN_DOWN(res_base - size, align);
		}
	}

	log_debug("%s: Failed to allocate 0x%lx bytes below 0x%lx\n",
		  __func__, (ulong)size, (ulong)max_addr);

	return -1;
}

static int _lmb_alloc_addr(phys_addr_t base, phys_size_t size, u32 flags)
{
	long rgn;
	struct lmb_region *lmb_memory = lmb.available_mem.data;

	/* Check if the requested address is in one of the memory regions */
	rgn = lmb_overlap_checks(&lmb.available_mem, base, size,
				 LMB_NOOVERWRITE, true);
	if (rgn >= 0) {
		/*
		 * Check if the requested end address is in the same memory
		 * region we found.
		 */
		if (lmb_addrs_overlap(lmb_memory[rgn].base,
				      lmb_memory[rgn].size,
				      base + size - 1, 1))
			/* ok, reserve the memory */
			return lmb_reserve(base, size, flags);
	}

	return -EINVAL;
}

int lmb_alloc_mem(enum lmb_mem_type type, u64 align, phys_addr_t *addr,
		  phys_size_t size, u32 flags)
{
	int ret = -1;

	if (!size)
		return 0;

	if (!addr)
		return -EINVAL;

	switch (type) {
	case LMB_MEM_ALLOC_ANY:
		*addr = LMB_ALLOC_ANYWHERE;
		fallthrough;
	case LMB_MEM_ALLOC_MAX:
		ret = _lmb_alloc_base(size, align, addr, flags);
		break;
	case LMB_MEM_ALLOC_ADDR:
		ret = _lmb_alloc_addr(*addr, size, flags);
		break;
	default:
		log_debug("%s: Invalid memory allocation type requested %d\n",
			  __func__, type);
	}

	return ret;
}

/* Return number of bytes from a given address that are free */
phys_size_t lmb_get_free_size(phys_addr_t addr)
{
	int i;
	long rgn;
	struct lmb_region *lmb_used = lmb.used_mem.data;
	struct lmb_region *lmb_memory = lmb.available_mem.data;

	/* check if the requested address is in the memory regions */
	rgn = lmb_overlap_checks(&lmb.available_mem, addr, 1, LMB_NOOVERWRITE,
				 true);
	if (rgn >= 0) {
		for (i = 0; i < lmb.used_mem.count; i++) {
			if (addr < lmb_used[i].base) {
				/* first reserved range > requested address */
				return lmb_used[i].base - addr;
			}
			if (lmb_used[i].base +
			    lmb_used[i].size > addr) {
				/* requested addr is in this reserved range */
				return 0;
			}
		}
		/* if we come here: no reserved ranges above requested addr */
		return lmb_memory[lmb.available_mem.count - 1].base +
		       lmb_memory[lmb.available_mem.count - 1].size - addr;
	}
	return 0;
}

int lmb_is_reserved_flags(phys_addr_t addr, int flags)
{
	int i;
	struct lmb_region *lmb_used = lmb.used_mem.data;

	for (i = 0; i < lmb.used_mem.count; i++) {
		phys_addr_t upper = lmb_used[i].base +
			lmb_used[i].size - 1;
		if (addr >= lmb_used[i].base && addr <= upper)
			return (lmb_used[i].flags & flags) == flags;
	}
	return 0;
}

static int lmb_setup(bool test)
{
	bool ret;

	ret = alist_init(&lmb.available_mem, sizeof(struct lmb_region),
			 (uint)LMB_ALIST_INITIAL_SIZE);
	if (!ret) {
		log_debug("Unable to initialise the list for LMB free memory\n");
		return -ENOMEM;
	}

	ret = alist_init(&lmb.used_mem, sizeof(struct lmb_region),
			 (uint)LMB_ALIST_INITIAL_SIZE);
	if (!ret) {
		log_debug("Unable to initialise the list for LMB used memory\n");
		return -ENOMEM;
	}

	lmb.test = test;

	return 0;
}

int lmb_init(void)
{
	int ret;

	ret = lmb_setup(false);
	if (ret) {
		log_info("Unable to init LMB\n");
		return ret;
	}

	lmb_add_memory();

	/* Reserve the U-Boot image region once U-Boot has relocated */
	if (xpl_phase() == PHASE_SPL)
		lmb_reserve_common_spl();
	else if (xpl_phase() == PHASE_BOARD_R)
		lmb_reserve_common((void *)gd->fdt_blob);

	return 0;
}

struct lmb *lmb_get(void)
{
	return &lmb;
}

#if CONFIG_IS_ENABLED(UNIT_TEST)
int lmb_push(struct lmb *store)
{
	int ret;

	*store = lmb;
	ret = lmb_setup(true);
	if (ret)
		return ret;

	return 0;
}

void lmb_pop(struct lmb *store)
{
	alist_uninit(&lmb.available_mem);
	alist_uninit(&lmb.used_mem);
	lmb = *store;
}
#endif /* UNIT_TEST */
