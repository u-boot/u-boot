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

#define LMB_ALLOC_ANYWHERE      0
#define LMB_ALIST_INITIAL_SIZE  4

/**
 * enum lmb_flags - definition of memory region attributes
 * @LMB_NONE: no special request
 * @LMB_NOMAP: don't add to mmu configuration
 * @LMB_NOOVERWRITE: the memory region cannot be overwritten/re-reserved
 * @LMB_NONOTIFY: do not notify other modules of changes to this memory region
 */
enum lmb_flags {
	LMB_NONE		= 0,
	LMB_NOMAP		= BIT(1),
	LMB_NOOVERWRITE		= BIT(2),
	LMB_NONOTIFY		= BIT(3),
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
 * @test:	Is structure being used for LMB tests
 */
struct lmb {
	struct alist free_mem;
	struct alist used_mem;
	bool test;
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

phys_addr_t lmb_alloc_base_flags(phys_size_t size, ulong align,
				 phys_addr_t max_addr, uint flags);

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
				 uint flags);

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
 * Returns: 0 on success, negative error code on failure
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
 * Returns: 0 if the region addition was successful, -1 on failure
 */
long io_lmb_add(struct lmb *io_lmb, phys_addr_t base, phys_size_t size);

/**
 * io_lmb_alloc() - Allocate specified IO memory address with specified alignment
 * @io_lmb: LMB to alloc from
 * @size: Size of the region requested
 * @align: Required address and size alignment
 *
 * Allocate a region of IO memory. The base parameter is used to specify the
 * base address of the requested region.
 *
 * Return: base IO address on success, 0 on error
 */
phys_addr_t io_lmb_alloc(struct lmb *io_lmb, phys_size_t size, ulong align);

/**
 * io_lmb_free() - Free up a region of IOVA space
 * @io_lmb: LMB to return the IO address space to
 * @base: Base Address of region to be freed
 * @size: Size of the region to be freed
 *
 * Free up a region of IOVA space.
 *
 * Return: 0 if successful, -1 on failure
 */
long io_lmb_free(struct lmb *io_lmb, phys_addr_t base, phys_size_t size);

#endif /* __KERNEL__ */

#endif /* _LINUX_LMB_H */
