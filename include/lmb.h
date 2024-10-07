/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _LINUX_LMB_H
#define _LINUX_LMB_H
#ifdef __KERNEL__

#include <alist.h>
#include <asm/types.h>
#include <asm/u-boot.h>
#include <linux/bitops.h>

/*
 * Logical memory blocks.
 *
 * Copyright (C) 2001 Peter Bergner, IBM Corp.
 */

/**
 * enum lmb_flags - definition of memory region attributes
 * @LMB_NONE: no special request
 * @LMB_NOMAP: don't add to mmu configuration
 */
enum lmb_flags {
	LMB_NONE		= 0,
	LMB_NOMAP		= BIT(1),
	LMB_NOOVERWRITE		= BIT(2),
};

/**
 * struct lmb_region - Description of one region.
 *
 * @base:	Base address of the region.
 * @size:	Size of the region
 * @flags:	memory region attributes
 */
struct lmb_region {
	phys_addr_t base;
	phys_size_t size;
	enum lmb_flags flags;
};

/**
 * struct lmb - The LMB structure
 *
 * @free_mem:	List of free memory regions
 * @used_mem:	List of used/reserved memory regions
 */
struct lmb {
	struct alist free_mem;
	struct alist used_mem;
};

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
int lmb_init(void);

/**
 * lmb_add_memory() - Add memory range for LMB allocations
 *
 * Add the entire available memory range to the pool of memory that
 * can be used by the LMB module for allocations.
 *
 * Return: None
 */
void lmb_add_memory(void);

long lmb_add(phys_addr_t base, phys_size_t size);
long lmb_reserve(phys_addr_t base, phys_size_t size);
/**
 * lmb_reserve_flags - Reserve one region with a specific flags bitfield.
 *
 * @base:	base address of the memory region
 * @size:	size of the memory region
 * @flags:	flags for the memory region
 * Return:	0 if OK, > 0 for coalesced region or a negative error code.
 */
long lmb_reserve_flags(phys_addr_t base, phys_size_t size,
		       enum lmb_flags flags);
phys_addr_t lmb_alloc(phys_size_t size, ulong align);
phys_addr_t lmb_alloc_base(phys_size_t size, ulong align, phys_addr_t max_addr);
phys_addr_t lmb_alloc_addr(phys_addr_t base, phys_size_t size);
phys_size_t lmb_get_free_size(phys_addr_t addr);

/**
 * lmb_is_reserved_flags() - test if address is in reserved region with flag bits set
 *
 * The function checks if a reserved region comprising @addr exists which has
 * all flag bits set which are set in @flags.
 *
 * @addr:	address to be tested
 * @flags:	bitmap with bits to be tested
 * Return:	1 if matching reservation exists, 0 otherwise
 */
int lmb_is_reserved_flags(phys_addr_t addr, int flags);

long lmb_free(phys_addr_t base, phys_size_t size);

void lmb_dump_all(void);
void lmb_dump_all_force(void);

struct lmb *lmb_get(void);
int lmb_push(struct lmb *store);
void lmb_pop(struct lmb *store);

static inline int lmb_read_check(phys_addr_t addr, phys_size_t len)
{
	return lmb_alloc_addr(addr, len) == addr ? 0 : -1;
}

#endif /* __KERNEL__ */

#endif /* _LINUX_LMB_H */
