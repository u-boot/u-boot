/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _LINUX_LMB_H
#define _LINUX_LMB_H
#ifdef __KERNEL__

#include <asm/types.h>
#include <asm/u-boot.h>

/*
 * Logical memory blocks.
 *
 * Copyright (C) 2001 Peter Bergner, IBM Corp.
 */

/**
 * struct lmb_property - Description of one region.
 *
 * @base: Base address of the region.
 * @size: Size of the region
 */
struct lmb_property {
	phys_addr_t base;
	phys_size_t size;
};

/**
 * struct lmb_region - Description of a set of region.
 *
 * @cnt: Number of regions.
 * @max: Size of the region array, max value of cnt.
 * @region: Array of the region properties
 */
struct lmb_region {
	unsigned long cnt;
	unsigned long max;
#if IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
	struct lmb_property region[CONFIG_LMB_MAX_REGIONS];
#else
	struct lmb_property *region;
#endif
};

/**
 * struct lmb - Logical memory block handle.
 *
 * Clients provide storage for Logical memory block (lmb) handles.
 * The content of the structure is managed by the lmb library.
 * A lmb struct is  initialized by lmb_init() functions.
 * The lmb struct is passed to all other lmb APIs.
 *
 * @memory: Description of memory regions.
 * @reserved: Description of reserved regions.
 * @memory_regions: Array of the memory regions (statically allocated)
 * @reserved_regions: Array of the reserved regions (statically allocated)
 */
struct lmb {
	struct lmb_region memory;
	struct lmb_region reserved;
#if !IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
	struct lmb_property memory_regions[CONFIG_LMB_MEMORY_REGIONS];
	struct lmb_property reserved_regions[CONFIG_LMB_RESERVED_REGIONS];
#endif
};

extern void lmb_init(struct lmb *lmb);
extern void lmb_init_and_reserve(struct lmb *lmb, struct bd_info *bd,
				 void *fdt_blob);
extern void lmb_init_and_reserve_range(struct lmb *lmb, phys_addr_t base,
				       phys_size_t size, void *fdt_blob);
extern long lmb_add(struct lmb *lmb, phys_addr_t base, phys_size_t size);
extern long lmb_reserve(struct lmb *lmb, phys_addr_t base, phys_size_t size);
extern phys_addr_t lmb_alloc(struct lmb *lmb, phys_size_t size, ulong align);
extern phys_addr_t lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align,
			    phys_addr_t max_addr);
extern phys_addr_t __lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align,
			      phys_addr_t max_addr);
extern phys_addr_t lmb_alloc_addr(struct lmb *lmb, phys_addr_t base,
				  phys_size_t size);
extern phys_size_t lmb_get_free_size(struct lmb *lmb, phys_addr_t addr);
extern int lmb_is_reserved(struct lmb *lmb, phys_addr_t addr);
extern long lmb_free(struct lmb *lmb, phys_addr_t base, phys_size_t size);

extern void lmb_dump_all(struct lmb *lmb);
extern void lmb_dump_all_force(struct lmb *lmb);

static inline phys_size_t
lmb_size_bytes(struct lmb_region *type, unsigned long region_nr)
{
	return type->region[region_nr].size;
}

void board_lmb_reserve(struct lmb *lmb);
void arch_lmb_reserve(struct lmb *lmb);

#endif /* __KERNEL__ */

#endif /* _LINUX_LMB_H */
