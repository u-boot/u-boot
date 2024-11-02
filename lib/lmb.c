// SPDX-License-Identifier: GPL-2.0+
/*
 * Procedures for maintaining information about logical memory blocks.
 *
 * Peter Bergner, IBM Corp.	June 2001.
 * Copyright (C) 2001 Peter Bergner.
 */

#include <alist.h>
#include <efi_loader.h>
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

#define MAP_OP_RESERVE		(u8)0x1
#define MAP_OP_FREE		(u8)0x2
#define MAP_OP_ADD		(u8)0x3

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

static long lmb_regions_overlap(struct alist *lmb_rgn_lst, unsigned long r1,
				unsigned long r2)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;

	phys_addr_t base1 = rgn[r1].base;
	phys_size_t size1 = rgn[r1].size;
	phys_addr_t base2 = rgn[r2].base;
	phys_size_t size2 = rgn[r2].size;

	return lmb_addrs_overlap(base1, size1, base2, size2);
}

static long lmb_regions_adjacent(struct alist *lmb_rgn_lst, unsigned long r1,
				 unsigned long r2)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;

	phys_addr_t base1 = rgn[r1].base;
	phys_size_t size1 = rgn[r1].size;
	phys_addr_t base2 = rgn[r2].base;
	phys_size_t size2 = rgn[r2].size;
	return lmb_addrs_adjacent(base1, size1, base2, size2);
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

/*Assumption : base addr of region 1 < base addr of region 2*/
static void lmb_fix_over_lap_regions(struct alist *lmb_rgn_lst,
				     unsigned long r1, unsigned long r2)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;

	phys_addr_t base1 = rgn[r1].base;
	phys_size_t size1 = rgn[r1].size;
	phys_addr_t base2 = rgn[r2].base;
	phys_size_t size2 = rgn[r2].size;

	if (base1 + size1 > base2 + size2) {
		printf("This will not be a case any time\n");
		return;
	}
	rgn[r1].size = base2 + size2 - base1;
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
 *
 * Returns: 0 if the region addition successful, -1 on failure
 */
static long lmb_add_region_flags(struct alist *lmb_rgn_lst, phys_addr_t base,
				 phys_size_t size, enum lmb_flags flags)
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
		phys_size_t rgnflags = rgn[i].flags;
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
			rgn[i].base -= size;
			rgn[i].size += size;
			coalesced++;
			break;
		} else if (ret < 0) {
			if (flags != rgnflags)
				break;
			rgn[i].size += size;
			coalesced++;
			break;
		} else if (lmb_addrs_overlap(base, size, rgnbase, rgnsize)) {
			if (flags == LMB_NONE) {
				ret = lmb_resize_regions(lmb_rgn_lst, i, base,
							 size);
				if (ret < 0)
					return -1;

				coalesced++;
				break;
			} else {
				return -1;
			}
		}
	}

	if (lmb_rgn_lst->count && i < lmb_rgn_lst->count - 1) {
		rgn = lmb_rgn_lst->data;
		if (rgn[i].flags == rgn[i + 1].flags) {
			if (lmb_regions_adjacent(lmb_rgn_lst, i, i + 1)) {
				lmb_coalesce_regions(lmb_rgn_lst, i, i + 1);
				coalesced++;
			} else if (lmb_regions_overlap(lmb_rgn_lst, i, i + 1)) {
				/* fix overlapping area */
				lmb_fix_over_lap_regions(lmb_rgn_lst, i, i + 1);
				coalesced++;
			}
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

	rgnbegin = rgnend = 0; /* supress gcc warnings */
	rgn = lmb_rgn_lst->data;
	/* Find the region where (base, size) belongs to */
	for (i = 0; i < lmb_rgn_lst->count; i++) {
		rgnbegin = rgn[i].base;
		rgnend = rgnbegin + rgn[i].size - 1;

		if ((rgnbegin <= base) && (end <= rgnend))
			break;
	}

	/* Didn't find the region */
	if (i == lmb_rgn_lst->count)
		return -1;

	/* Check to see if we are removing entire region */
	if ((rgnbegin == base) && (rgnend == end)) {
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

static long lmb_overlaps_region(struct alist *lmb_rgn_lst, phys_addr_t base,
				phys_size_t size)
{
	unsigned long i;
	struct lmb_region *rgn = lmb_rgn_lst->data;

	for (i = 0; i < lmb_rgn_lst->count; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;
		if (lmb_addrs_overlap(base, size, rgnbase, rgnsize))
			break;
	}

	return (i < lmb_rgn_lst->count) ? i : -1;
}

static phys_addr_t lmb_align_down(phys_addr_t addr, phys_size_t size)
{
	return addr & ~(size - 1);
}

/*
 * IOVA LMB memory maps using lmb pointers instead of the global LMB memory map.
 */

int io_lmb_setup(struct lmb *io_lmb)
{
	int ret;

	ret = alist_init(&io_lmb->free_mem, sizeof(struct lmb_region),
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
	alist_uninit(&io_lmb->free_mem);
	alist_uninit(&io_lmb->used_mem);
}

long io_lmb_add(struct lmb *io_lmb, phys_addr_t base, phys_size_t size)
{
	return lmb_add_region_flags(&io_lmb->free_mem, base, size, LMB_NONE);
}

/* derived and simplified from _lmb_alloc_base() */
phys_addr_t io_lmb_alloc(struct lmb *io_lmb, phys_size_t size, ulong align)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;
	struct lmb_region *lmb_used = io_lmb->used_mem.data;
	struct lmb_region *lmb_memory = io_lmb->free_mem.data;

	for (i = io_lmb->free_mem.count - 1; i >= 0; i--) {
		phys_addr_t lmbbase = lmb_memory[i].base;
		phys_size_t lmbsize = lmb_memory[i].size;

		if (lmbsize < size)
			continue;
		base = lmb_align_down(lmbbase + lmbsize - size, align);

		while (base && lmbbase <= base) {
			rgn = lmb_overlaps_region(&io_lmb->used_mem, base, size);
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
			base = lmb_align_down(res_base - size, align);
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

static bool lmb_should_notify(enum lmb_flags flags)
{
	return !lmb.test && !(flags & LMB_NONOTIFY) &&
		CONFIG_IS_ENABLED(EFI_LOADER);
}

static int lmb_map_update_notify(phys_addr_t addr, phys_size_t size, u8 op,
				 enum lmb_flags flags)
{
	u64 efi_addr;
	u64 pages;
	efi_status_t status;

	if (op != MAP_OP_RESERVE && op != MAP_OP_FREE && op != MAP_OP_ADD) {
		log_err("Invalid map update op received (%d)\n", op);
		return -1;
	}

	if (!lmb_should_notify(flags))
		return 0;

	efi_addr = (uintptr_t)map_sysmem(addr, 0);
	pages = efi_size_in_pages(size + (efi_addr & EFI_PAGE_MASK));
	efi_addr &= ~EFI_PAGE_MASK;

	status = efi_add_memory_map_pg(efi_addr, pages,
				       op == MAP_OP_RESERVE ?
				       EFI_BOOT_SERVICES_DATA :
				       EFI_CONVENTIONAL_MEMORY,
				       false);
	if (status != EFI_SUCCESS) {
		log_err("%s: LMB Map notify failure %lu\n", __func__,
			status & ~EFI_ERROR_MASK);
		return -1;
	}
	unmap_sysmem((void *)(uintptr_t)efi_addr);

	return 0;
}

static void lmb_print_region_flags(enum lmb_flags flags)
{
	const char *flag_str[] = { "none", "no-map", "no-overwrite", "no-notify" };
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
	enum lmb_flags flags;
	int i;

	printf(" %s.count = 0x%x\n", name, lmb_rgn_lst->count);

	for (i = 0; i < lmb_rgn_lst->count; i++) {
		base = rgn[i].base;
		size = rgn[i].size;
		end = base + size - 1;
		flags = rgn[i].flags;

		printf(" %s[%d]\t[0x%llx-0x%llx], 0x%08llx bytes flags: ",
		       name, i, base, end, size);
		lmb_print_region_flags(flags);
	}
}

void lmb_dump_all_force(void)
{
	printf("lmb_dump_all:\n");
	lmb_dump_region(&lmb.free_mem, "memory");
	lmb_dump_region(&lmb.used_mem, "reserved");
}

void lmb_dump_all(void)
{
#ifdef DEBUG
	lmb_dump_all_force();
#endif
}

static void lmb_reserve_uboot_region(void)
{
	int bank;
	ulong end, bank_end;
	phys_addr_t rsv_start;

	rsv_start = gd->start_addr_sp - CONFIG_STACK_SIZE;
	end = gd->ram_top;

	/*
	 * Reserve memory from aligned address below the bottom of U-Boot stack
	 * until end of RAM area to prevent LMB from overwriting that memory.
	 */
	debug("## Current stack ends at 0x%08lx ", (ulong)rsv_start);

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

		lmb_reserve_flags(rsv_start, bank_end - rsv_start + 1,
				  LMB_NOOVERWRITE);

		if (gd->flags & GD_FLG_SKIP_RELOC)
			lmb_reserve_flags((phys_addr_t)(uintptr_t)_start,
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
		lmb_reserve_flags(rsv_start, rsv_size, LMB_NOOVERWRITE);
	}

	if (IS_ENABLED(CONFIG_SPL_SEPARATE_BSS)) {
		/* Reserve the bss region */
		rsv_start = (phys_addr_t)(uintptr_t)__bss_start;
		rsv_size = (phys_addr_t)(uintptr_t)__bss_end -
			(phys_addr_t)(uintptr_t)__bss_start;
		lmb_reserve_flags(rsv_start, rsv_size, LMB_NOOVERWRITE);
	}
}

/**
 * lmb_add_memory() - Add memory range for LMB allocations
 *
 * Add the entire available memory range to the pool of memory that
 * can be used by the LMB module for allocations.
 *
 * Return: None
 */
void lmb_add_memory(void)
{
	int i;
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
		if (size) {
			lmb_add(bd->bi_dram[i].start, size);

			/*
			 * Reserve memory above ram_top as
			 * no-overwrite so that it cannot be
			 * allocated
			 */
			if (bd->bi_dram[i].start >= ram_top)
				lmb_reserve_flags(bd->bi_dram[i].start, size,
						  LMB_NOOVERWRITE);
		}
	}
}

static long lmb_add_region(struct alist *lmb_rgn_lst, phys_addr_t base,
			   phys_size_t size)
{
	return lmb_add_region_flags(lmb_rgn_lst, base, size, LMB_NONE);
}

/* This routine may be called with relocation disabled. */
long lmb_add(phys_addr_t base, phys_size_t size)
{
	long ret;
	struct alist *lmb_rgn_lst = &lmb.free_mem;

	ret = lmb_add_region(lmb_rgn_lst, base, size);
	if (ret)
		return ret;

	return lmb_map_update_notify(base, size, MAP_OP_ADD, LMB_NONE);
}

/**
 * lmb_free_flags() - Free up a region of memory
 * @base: Base Address of region to be freed
 * @size: Size of the region to be freed
 * @flags: Memory region attributes
 *
 * Free up a region of memory.
 *
 * Return: 0 if successful, -1 on failure
 */
long lmb_free_flags(phys_addr_t base, phys_size_t size,
		    uint flags)
{
	long ret;

	ret = _lmb_free(&lmb.used_mem, base, size);
	if (ret < 0)
		return ret;

	return lmb_map_update_notify(base, size, MAP_OP_FREE, flags);
}

long lmb_free(phys_addr_t base, phys_size_t size)
{
	return lmb_free_flags(base, size, LMB_NONE);
}

long lmb_reserve_flags(phys_addr_t base, phys_size_t size, enum lmb_flags flags)
{
	long ret = 0;
	struct alist *lmb_rgn_lst = &lmb.used_mem;

	ret = lmb_add_region_flags(lmb_rgn_lst, base, size, flags);
	if (ret)
		return ret;

	return lmb_map_update_notify(base, size, MAP_OP_RESERVE, flags);
}

long lmb_reserve(phys_addr_t base, phys_size_t size)
{
	return lmb_reserve_flags(base, size, LMB_NONE);
}

static phys_addr_t _lmb_alloc_base(phys_size_t size, ulong align,
				    phys_addr_t max_addr, enum lmb_flags flags)
{
	int ret;
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;
	struct lmb_region *lmb_used = lmb.used_mem.data;
	struct lmb_region *lmb_memory = lmb.free_mem.data;

	for (i = lmb.free_mem.count - 1; i >= 0; i--) {
		phys_addr_t lmbbase = lmb_memory[i].base;
		phys_size_t lmbsize = lmb_memory[i].size;

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
			rgn = lmb_overlaps_region(&lmb.used_mem, base, size);
			if (rgn < 0) {
				/* This area isn't reserved, take it */
				if (lmb_add_region_flags(&lmb.used_mem, base,
							 size, flags))
					return 0;

				ret = lmb_map_update_notify(base, size,
							    MAP_OP_RESERVE,
							    flags);
				if (ret)
					return ret;

				return base;
			}

			res_base = lmb_used[rgn].base;
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

	alloc = _lmb_alloc_base(size, align, max_addr, LMB_NONE);

	if (alloc == 0)
		printf("ERROR: Failed to allocate 0x%lx bytes below 0x%lx.\n",
		       (ulong)size, (ulong)max_addr);

	return alloc;
}

/**
 * lmb_alloc_base_flags() - Allocate specified memory region with specified attributes
 * @size: Size of the region requested
 * @align: Alignment of the memory region requested
 * @max_addr: Maximum address of the requested region
 * @flags: Memory region attributes to be set
 *
 * Allocate a region of memory with the attributes specified through the
 * parameter. The max_addr parameter is used to specify the maximum address
 * below which the requested region should be allocated.
 *
 * Return: base address on success, 0 on error
 */
phys_addr_t lmb_alloc_base_flags(phys_size_t size, ulong align,
				 phys_addr_t max_addr, uint flags)
{
	phys_addr_t alloc;

	alloc = _lmb_alloc_base(size, align, max_addr, flags);

	if (alloc == 0)
		printf("ERROR: Failed to allocate 0x%lx bytes below 0x%lx.\n",
		       (ulong)size, (ulong)max_addr);

	return alloc;
}

static phys_addr_t _lmb_alloc_addr(phys_addr_t base, phys_size_t size,
				    enum lmb_flags flags)
{
	long rgn;
	struct lmb_region *lmb_memory = lmb.free_mem.data;

	/* Check if the requested address is in one of the memory regions */
	rgn = lmb_overlaps_region(&lmb.free_mem, base, size);
	if (rgn >= 0) {
		/*
		 * Check if the requested end address is in the same memory
		 * region we found.
		 */
		if (lmb_addrs_overlap(lmb_memory[rgn].base,
				      lmb_memory[rgn].size,
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
	return _lmb_alloc_addr(base, size, LMB_NONE);
}

/**
 * lmb_alloc_addr_flags() - Allocate specified memory address with specified attributes
 * @base: Base Address requested
 * @size: Size of the region requested
 * @flags: Memory region attributes to be set
 *
 * Allocate a region of memory with the attributes specified through the
 * parameter. The base parameter is used to specify the base address
 * of the requested region.
 *
 * Return: base address on success, 0 on error
 */
phys_addr_t lmb_alloc_addr_flags(phys_addr_t base, phys_size_t size,
				 uint flags)
{
	return _lmb_alloc_addr(base, size, flags);
}

/* Return number of bytes from a given address that are free */
phys_size_t lmb_get_free_size(phys_addr_t addr)
{
	int i;
	long rgn;
	struct lmb_region *lmb_used = lmb.used_mem.data;
	struct lmb_region *lmb_memory = lmb.free_mem.data;

	/* check if the requested address is in the memory regions */
	rgn = lmb_overlaps_region(&lmb.free_mem, addr, 1);
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
		return lmb_memory[lmb.free_mem.count - 1].base +
		       lmb_memory[lmb.free_mem.count - 1].size - addr;
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

	ret = alist_init(&lmb.free_mem, sizeof(struct lmb_region),
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

/**
 * lmb_init() - Initialise the LMB module
 *
 * Initialise the LMB lists needed for keeping the memory map. There
 * are two lists, in form of alloced list data structure. One for the
 * available memory, and one for the used memory. Initialise the two
 * lists as part of board init. Add memory to the available memory
 * list and reserve common areas by adding them to the used memory
 * list.
 *
 * Return: 0 on success, -ve on error
 */
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
	alist_uninit(&lmb.free_mem);
	alist_uninit(&lmb.used_mem);
	lmb = *store;
}
#endif /* UNIT_TEST */
