/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SANDBOX_ASM_IO_H
#define __SANDBOX_ASM_IO_H

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 */
#define MAP_NOCACHE	(0)
#define MAP_WRCOMBINE	(0)
#define MAP_WRBACK	(0)
#define MAP_WRTHROUGH	(0)

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags);

/*
 * Take down a mapping set up by map_physmem().
 */
static inline void unmap_physmem(void *vaddr, unsigned long flags)
{

}

/* For sandbox, we want addresses to point into our RAM buffer */
static inline void *map_sysmem(phys_addr_t paddr, unsigned long len)
{
	return map_physmem(paddr, len, MAP_WRBACK);
}

static inline void unmap_sysmem(const void *vaddr)
{
}

/* Map from a pointer to our RAM buffer */
phys_addr_t map_to_sysmem(const void *ptr);

/* Define nops for sandbox I/O access */
#define readb(addr) 0
#define readw(addr) 0
#define readl(addr) 0
#define writeb(v, addr)
#define writew(v, addr)
#define writel(v, addr)

#include <iotrace.h>

#endif
