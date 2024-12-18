/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Logical memory blocks.
 *
 * Copyright (C) 2001 Peter Bergner, IBM Corp.
 */

#ifndef _LINUX_LMB_H
#define _LINUX_LMB_H

#ifdef __KERNEL__

#include <alist.h>
#include <asm/types.h>
#include <asm/u-boot.h>
#include <linux/bitops.h>

#define LMB_ALLOC_ANYWHERE	0
#define LMB_ALIST_INITIAL_SIZE	4

/**
 * DOC: Memory region attribute flags.
 *
 * %LMB_NONE: No special request
 * %LMB_NOMAP: Don't add to MMU configuration
 * %LMB_NOOVERWRITE: The memory region cannot be overwritten/re-reserved
 * %LMB_NONOTIFY: Do not notify other modules of changes to this memory region
 */
#define LMB_NONE 0
#define LMB_NOMAP BIT(0)
#define LMB_NOOVERWRITE BIT(1)
#define LMB_NONOTIFY BIT(2)

/**
 * struct lmb_region - Description of one region
 * @base: Base address of the region
 * @size: Size of the region
 * @flags: Memory region attributes
 */
struct lmb_region {
	phys_addr_t base;
	phys_size_t size;
	u32 flags;
};

/**
 * struct lmb - The LMB structure
 * @available_mem: List of memory available to LMB
 * @used_mem: List of used/reserved memory regions
 * @test: Is structure being used for LMB tests
 */
struct lmb {
	struct alist available_mem;
	struct alist used_mem;
	bool test;
};

/**
 * lmb_init() - Initialise the LMB module.
 *
 * Return: 0 on success, negative error code on failure.
 *
 * Initialise the LMB lists needed for keeping the memory map. There
 * are two lists, in form of allocated list data structure. One for the
 * available memory, and one for the used memory. Initialise the two
 * lists as part of board init. Add memory to the available memory
 * list and reserve common areas by adding them to the used memory
 * list.
 */
int lmb_init(void);

/**
 * lmb_add_memory() - Add memory range for LMB allocations.
 *
 * Add the entire available memory range to the pool of memory that
 * can be used by the LMB module for allocations.
 */
void lmb_add_memory(void);

long lmb_add(phys_addr_t base, phys_size_t size);

/**
 * lmb_reserve() - Reserve one region with a specific flags bitfield
 * @base: Base address of the memory region
 * @size: Size of the memory region
 * @flags: Flags for the memory region
 *
 * Return:
 * * %0		- Added successfully, or it's already added (only if LMB_NONE)
 * * %-EEXIST	- The region is already added, and flags != LMB_NONE
 * * %-1	- Failure
 */
long lmb_reserve(phys_addr_t base, phys_size_t size, u32 flags);

phys_addr_t lmb_alloc(phys_size_t size, ulong align);
phys_addr_t lmb_alloc_base(phys_size_t size, ulong align, phys_addr_t max_addr);
phys_addr_t lmb_alloc_addr(phys_addr_t base, phys_size_t size);
phys_size_t lmb_get_free_size(phys_addr_t addr);

/**
 * lmb_alloc_base_flags() - Allocate specified memory region with specified
 *			    attributes
 * @size: Size of the region requested
 * @align: Alignment of the memory region requested
 * @max_addr: Maximum address of the requested region
 * @flags: Memory region attributes to be set
 *
 * Allocate a region of memory with the attributes specified through the
 * parameter. The max_addr parameter is used to specify the maximum address
 * below which the requested region should be allocated.
 *
 * Return: Base address on success, 0 on error.
 */
phys_addr_t lmb_alloc_base_flags(phys_size_t size, ulong align,
				 phys_addr_t max_addr, uint flags);

/**
 * lmb_alloc_addr_flags() - Allocate specified memory address with specified
 *			    attributes
 * @base: Base Address requested
 * @size: Size of the region requested
 * @flags: Memory region attributes to be set
 *
 * Allocate a region of memory with the attributes specified through the
 * parameter. The base parameter is used to specify the base address
 * of the requested region.
 *
 * Return: Base address on success, 0 on error.
 */
phys_addr_t lmb_alloc_addr_flags(phys_addr_t base, phys_size_t size,
				 uint flags);

/**
 * lmb_is_reserved_flags() - Test if address is in reserved region with flag
 *			     bits set
 * @addr: Address to be tested
 * @flags: Bitmap with bits to be tested
 *
 * The function checks if a reserved region comprising @addr exists which has
 * all flag bits set which are set in @flags.
 *
 * Return: 1 if matching reservation exists, 0 otherwise.
 */
int lmb_is_reserved_flags(phys_addr_t addr, int flags);

/**
 * lmb_free_flags() - Free up a region of memory
 * @base: Base Address of region to be freed
 * @size: Size of the region to be freed
 * @flags: Memory region attributes
 *
 * Return: 0 on success, negative error code on failure.
 */
long lmb_free_flags(phys_addr_t base, phys_size_t size, uint flags);

long lmb_free(phys_addr_t base, phys_size_t size);

void lmb_dump_all(void);
void lmb_dump_all_force(void);

void lmb_arch_add_memory(void);

struct lmb *lmb_get(void);
int lmb_push(struct lmb *store);
void lmb_pop(struct lmb *store);

static inline int lmb_read_check(phys_addr_t addr, phys_size_t len)
{
	return lmb_alloc_addr(addr, len) == addr ? 0 : -1;
}

/**
 * io_lmb_setup() - Initialize LMB struct
 * @io_lmb: IO LMB to initialize
 *
 * Return: 0 on success, negative error code on failure.
 */
int io_lmb_setup(struct lmb *io_lmb);

/**
 * io_lmb_teardown() - Tear LMB struct down
 * @io_lmb: IO LMB to teardown
 */
void io_lmb_teardown(struct lmb *io_lmb);

/**
 * io_lmb_add() - Add an IOVA range for allocations
 * @io_lmb: LMB to add the space to
 * @base: Base Address of region to add
 * @size: Size of the region to add
 *
 * Add the IOVA space [base, base + size] to be managed by io_lmb.
 *
 * Return: 0 on success, negative error code on failure.
 */
long io_lmb_add(struct lmb *io_lmb, phys_addr_t base, phys_size_t size);

/**
 * io_lmb_alloc() - Allocate specified IO memory address with specified
 *		    alignment
 * @io_lmb: LMB to alloc from
 * @size: Size of the region requested
 * @align: Required address and size alignment
 *
 * Allocate a region of IO memory. The base parameter is used to specify the
 * base address of the requested region.
 *
 * Return: Base IO address on success, 0 on error.
 */
phys_addr_t io_lmb_alloc(struct lmb *io_lmb, phys_size_t size, ulong align);

/**
 * io_lmb_free() - Free up a region of IOVA space
 * @io_lmb: LMB to return the IO address space to
 * @base: Base Address of region to be freed
 * @size: Size of the region to be freed
 *
 * Return: 0 on success, negative error code on failure.
 */
long io_lmb_free(struct lmb *io_lmb, phys_addr_t base, phys_size_t size);

#endif /* __KERNEL__ */

#endif /* _LINUX_LMB_H */
