/*
 *  EFI application memory management
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <libfdt_env.h>
#include <linux/list_sort.h>
#include <inttypes.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

struct efi_mem_list {
	struct list_head link;
	struct efi_mem_desc desc;
};

#define EFI_CARVE_NO_OVERLAP		-1
#define EFI_CARVE_LOOP_AGAIN		-2
#define EFI_CARVE_OVERLAPS_NONRAM	-3

/* This list contains all memory map items */
LIST_HEAD(efi_mem);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
void *efi_bounce_buffer;
#endif

/*
 * U-Boot services each EFI AllocatePool request as a separate
 * (multiple) page allocation.  We have to track the number of pages
 * to be able to free the correct amount later.
 * EFI requires 8 byte alignment for pool allocations, so we can
 * prepend each allocation with an 64 bit header tracking the
 * allocation size, and hand out the remainder to the caller.
 */
struct efi_pool_allocation {
	u64 num_pages;
	char data[];
};

/*
 * Sorts the memory list from highest address to lowest address
 *
 * When allocating memory we should always start from the highest
 * address chunk, so sort the memory list such that the first list
 * iterator gets the highest address and goes lower from there.
 */
static int efi_mem_cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct efi_mem_list *mema = list_entry(a, struct efi_mem_list, link);
	struct efi_mem_list *memb = list_entry(b, struct efi_mem_list, link);

	if (mema->desc.physical_start == memb->desc.physical_start)
		return 0;
	else if (mema->desc.physical_start < memb->desc.physical_start)
		return 1;
	else
		return -1;
}

static void efi_mem_sort(void)
{
	list_sort(NULL, &efi_mem, efi_mem_cmp);
}

/*
 * Unmaps all memory occupied by the carve_desc region from the
 * list entry pointed to by map.
 *
 * Returns EFI_CARVE_NO_OVERLAP if the regions don't overlap.
 * Returns EFI_CARVE_OVERLAPS_NONRAM if the carve and map overlap,
 *    and the map contains anything but free ram.
 *    (only when overlap_only_ram is true)
 * Returns EFI_CARVE_LOOP_AGAIN if the mapping list should be traversed
 *    again, as it has been altered
 * Returns the number of overlapping pages. The pages are removed from
 *     the mapping list.
 *
 * In case of EFI_CARVE_OVERLAPS_NONRAM it is the callers responsibility
 * to readd the already carved out pages to the mapping.
 */
static int efi_mem_carve_out(struct efi_mem_list *map,
			     struct efi_mem_desc *carve_desc,
			     bool overlap_only_ram)
{
	struct efi_mem_list *newmap;
	struct efi_mem_desc *map_desc = &map->desc;
	uint64_t map_start = map_desc->physical_start;
	uint64_t map_end = map_start + (map_desc->num_pages << EFI_PAGE_SHIFT);
	uint64_t carve_start = carve_desc->physical_start;
	uint64_t carve_end = carve_start +
			     (carve_desc->num_pages << EFI_PAGE_SHIFT);

	/* check whether we're overlapping */
	if ((carve_end <= map_start) || (carve_start >= map_end))
		return EFI_CARVE_NO_OVERLAP;

	/* We're overlapping with non-RAM, warn the caller if desired */
	if (overlap_only_ram && (map_desc->type != EFI_CONVENTIONAL_MEMORY))
		return EFI_CARVE_OVERLAPS_NONRAM;

	/* Sanitize carve_start and carve_end to lie within our bounds */
	carve_start = max(carve_start, map_start);
	carve_end = min(carve_end, map_end);

	/* Carving at the beginning of our map? Just move it! */
	if (carve_start == map_start) {
		if (map_end == carve_end) {
			/* Full overlap, just remove map */
			list_del(&map->link);
			free(map);
		} else {
			map->desc.physical_start = carve_end;
			map->desc.num_pages = (map_end - carve_end)
					      >> EFI_PAGE_SHIFT;
		}

		return (carve_end - carve_start) >> EFI_PAGE_SHIFT;
	}

	/*
	 * Overlapping maps, just split the list map at carve_start,
	 * it will get moved or removed in the next iteration.
	 *
	 * [ map_desc |__carve_start__| newmap ]
	 */

	/* Create a new map from [ carve_start ... map_end ] */
	newmap = calloc(1, sizeof(*newmap));
	newmap->desc = map->desc;
	newmap->desc.physical_start = carve_start;
	newmap->desc.num_pages = (map_end - carve_start) >> EFI_PAGE_SHIFT;
	/* Insert before current entry (descending address order) */
	list_add_tail(&newmap->link, &map->link);

	/* Shrink the map to [ map_start ... carve_start ] */
	map_desc->num_pages = (carve_start - map_start) >> EFI_PAGE_SHIFT;

	return EFI_CARVE_LOOP_AGAIN;
}

uint64_t efi_add_memory_map(uint64_t start, uint64_t pages, int memory_type,
			    bool overlap_only_ram)
{
	struct list_head *lhandle;
	struct efi_mem_list *newlist;
	bool carve_again;
	uint64_t carved_pages = 0;

	debug("%s: 0x%" PRIx64 " 0x%" PRIx64 " %d %s\n", __func__,
	      start, pages, memory_type, overlap_only_ram ? "yes" : "no");

	if (!pages)
		return start;

	newlist = calloc(1, sizeof(*newlist));
	newlist->desc.type = memory_type;
	newlist->desc.physical_start = start;
	newlist->desc.virtual_start = start;
	newlist->desc.num_pages = pages;

	switch (memory_type) {
	case EFI_RUNTIME_SERVICES_CODE:
	case EFI_RUNTIME_SERVICES_DATA:
		newlist->desc.attribute = (1 << EFI_MEMORY_WB_SHIFT) |
					  (1ULL << EFI_MEMORY_RUNTIME_SHIFT);
		break;
	case EFI_MMAP_IO:
		newlist->desc.attribute = 1ULL << EFI_MEMORY_RUNTIME_SHIFT;
		break;
	default:
		newlist->desc.attribute = 1 << EFI_MEMORY_WB_SHIFT;
		break;
	}

	/* Add our new map */
	do {
		carve_again = false;
		list_for_each(lhandle, &efi_mem) {
			struct efi_mem_list *lmem;
			int r;

			lmem = list_entry(lhandle, struct efi_mem_list, link);
			r = efi_mem_carve_out(lmem, &newlist->desc,
					      overlap_only_ram);
			switch (r) {
			case EFI_CARVE_OVERLAPS_NONRAM:
				/*
				 * The user requested to only have RAM overlaps,
				 * but we hit a non-RAM region. Error out.
				 */
				return 0;
			case EFI_CARVE_NO_OVERLAP:
				/* Just ignore this list entry */
				break;
			case EFI_CARVE_LOOP_AGAIN:
				/*
				 * We split an entry, but need to loop through
				 * the list again to actually carve it.
				 */
				carve_again = true;
				break;
			default:
				/* We carved a number of pages */
				carved_pages += r;
				carve_again = true;
				break;
			}

			if (carve_again) {
				/* The list changed, we need to start over */
				break;
			}
		}
	} while (carve_again);

	if (overlap_only_ram && (carved_pages != pages)) {
		/*
		 * The payload wanted to have RAM overlaps, but we overlapped
		 * with an unallocated region. Error out.
		 */
		return 0;
	}

	/* Add our new map */
        list_add_tail(&newlist->link, &efi_mem);

	/* And make sure memory is listed in descending order */
	efi_mem_sort();

	return start;
}

static uint64_t efi_find_free_memory(uint64_t len, uint64_t max_addr)
{
	struct list_head *lhandle;

	list_for_each(lhandle, &efi_mem) {
		struct efi_mem_list *lmem = list_entry(lhandle,
			struct efi_mem_list, link);
		struct efi_mem_desc *desc = &lmem->desc;
		uint64_t desc_len = desc->num_pages << EFI_PAGE_SHIFT;
		uint64_t desc_end = desc->physical_start + desc_len;
		uint64_t curmax = min(max_addr, desc_end);
		uint64_t ret = curmax - len;

		/* We only take memory from free RAM */
		if (desc->type != EFI_CONVENTIONAL_MEMORY)
			continue;

		/* Out of bounds for max_addr */
		if ((ret + len) > max_addr)
			continue;

		/* Out of bounds for upper map limit */
		if ((ret + len) > desc_end)
			continue;

		/* Out of bounds for lower map limit */
		if (ret < desc->physical_start)
			continue;

		/* Return the highest address in this map within bounds */
		return ret;
	}

	return 0;
}

efi_status_t efi_allocate_pages(int type, int memory_type,
				unsigned long pages, uint64_t *memory)
{
	u64 len = pages << EFI_PAGE_SHIFT;
	efi_status_t r = EFI_SUCCESS;
	uint64_t addr;

	switch (type) {
	case 0:
		/* Any page */
		addr = efi_find_free_memory(len, gd->start_addr_sp);
		if (!addr) {
			r = EFI_NOT_FOUND;
			break;
		}
		break;
	case 1:
		/* Max address */
		addr = efi_find_free_memory(len, *memory);
		if (!addr) {
			r = EFI_NOT_FOUND;
			break;
		}
		break;
	case 2:
		/* Exact address, reserve it. The addr is already in *memory. */
		addr = *memory;
		break;
	default:
		/* UEFI doesn't specify other allocation types */
		r = EFI_INVALID_PARAMETER;
		break;
	}

	if (r == EFI_SUCCESS) {
		uint64_t ret;

		/* Reserve that map in our memory maps */
		ret = efi_add_memory_map(addr, pages, memory_type, true);
		if (ret == addr) {
			*memory = addr;
		} else {
			/* Map would overlap, bail out */
			r = EFI_OUT_OF_RESOURCES;
		}
	}

	return r;
}

void *efi_alloc(uint64_t len, int memory_type)
{
	uint64_t ret = 0;
	uint64_t pages = (len + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;
	efi_status_t r;

	r = efi_allocate_pages(0, memory_type, pages, &ret);
	if (r == EFI_SUCCESS)
		return (void*)(uintptr_t)ret;

	return NULL;
}

efi_status_t efi_free_pages(uint64_t memory, unsigned long pages)
{
	uint64_t r = 0;

	r = efi_add_memory_map(memory, pages, EFI_CONVENTIONAL_MEMORY, false);
	/* Merging of adjacent free regions is missing */

	if (r == memory)
		return EFI_SUCCESS;

	return EFI_NOT_FOUND;
}

efi_status_t efi_allocate_pool(int pool_type, unsigned long size,
			       void **buffer)
{
	efi_status_t r;
	efi_physical_addr_t t;
	u64 num_pages = (size + sizeof(u64) + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;

	if (size == 0) {
		*buffer = NULL;
		return EFI_SUCCESS;
	}

	r = efi_allocate_pages(0, pool_type, num_pages, &t);

	if (r == EFI_SUCCESS) {
		struct efi_pool_allocation *alloc = (void *)(uintptr_t)t;
		alloc->num_pages = num_pages;
		*buffer = alloc->data;
	}

	return r;
}

efi_status_t efi_free_pool(void *buffer)
{
	efi_status_t r;
	struct efi_pool_allocation *alloc;

	alloc = container_of(buffer, struct efi_pool_allocation, data);
	/* Sanity check, was the supplied address returned by allocate_pool */
	assert(((uintptr_t)alloc & EFI_PAGE_MASK) == 0);

	r = efi_free_pages((uintptr_t)alloc, alloc->num_pages);

	return r;
}

efi_status_t efi_get_memory_map(unsigned long *memory_map_size,
			       struct efi_mem_desc *memory_map,
			       unsigned long *map_key,
			       unsigned long *descriptor_size,
			       uint32_t *descriptor_version)
{
	ulong map_size = 0;
	int map_entries = 0;
	struct list_head *lhandle;
	unsigned long provided_map_size = *memory_map_size;

	list_for_each(lhandle, &efi_mem)
		map_entries++;

	map_size = map_entries * sizeof(struct efi_mem_desc);

	*memory_map_size = map_size;

	if (descriptor_size)
		*descriptor_size = sizeof(struct efi_mem_desc);

	if (descriptor_version)
		*descriptor_version = EFI_MEMORY_DESCRIPTOR_VERSION;

	if (provided_map_size < map_size)
		return EFI_BUFFER_TOO_SMALL;

	/* Copy list into array */
	if (memory_map) {
		/* Return the list in ascending order */
		memory_map = &memory_map[map_entries - 1];
		list_for_each(lhandle, &efi_mem) {
			struct efi_mem_list *lmem;

			lmem = list_entry(lhandle, struct efi_mem_list, link);
			*memory_map = lmem->desc;
			memory_map--;
		}
	}

	return EFI_SUCCESS;
}

__weak void efi_add_known_memory(void)
{
	int i;

	/* Add RAM */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		u64 ram_start = gd->bd->bi_dram[i].start;
		u64 ram_size = gd->bd->bi_dram[i].size;
		u64 start = (ram_start + EFI_PAGE_MASK) & ~EFI_PAGE_MASK;
		u64 pages = (ram_size + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;

		efi_add_memory_map(start, pages, EFI_CONVENTIONAL_MEMORY,
				   false);
	}
}

int efi_memory_init(void)
{
	unsigned long runtime_start, runtime_end, runtime_pages;
	unsigned long uboot_start, uboot_pages;
	unsigned long uboot_stack_size = 16 * 1024 * 1024;

	efi_add_known_memory();

	/* Add U-Boot */
	uboot_start = (gd->start_addr_sp - uboot_stack_size) & ~EFI_PAGE_MASK;
	uboot_pages = (gd->ram_top - uboot_start) >> EFI_PAGE_SHIFT;
	efi_add_memory_map(uboot_start, uboot_pages, EFI_LOADER_DATA, false);

	/* Add Runtime Services */
	runtime_start = (ulong)&__efi_runtime_start & ~EFI_PAGE_MASK;
	runtime_end = (ulong)&__efi_runtime_stop;
	runtime_end = (runtime_end + EFI_PAGE_MASK) & ~EFI_PAGE_MASK;
	runtime_pages = (runtime_end - runtime_start) >> EFI_PAGE_SHIFT;
	efi_add_memory_map(runtime_start, runtime_pages,
			   EFI_RUNTIME_SERVICES_CODE, false);

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	/* Request a 32bit 64MB bounce buffer region */
	uint64_t efi_bounce_buffer_addr = 0xffffffff;

	if (efi_allocate_pages(1, EFI_LOADER_DATA,
			       (64 * 1024 * 1024) >> EFI_PAGE_SHIFT,
			       &efi_bounce_buffer_addr) != EFI_SUCCESS)
		return -1;

	efi_bounce_buffer = (void*)(uintptr_t)efi_bounce_buffer_addr;
#endif

	return 0;
}
