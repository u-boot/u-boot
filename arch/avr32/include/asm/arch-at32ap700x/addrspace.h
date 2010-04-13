/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ASM_AVR32_ADDRSPACE_H
#define __ASM_AVR32_ADDRSPACE_H

#include <asm/types.h>

/* Memory segments when segmentation is enabled */
#define P0SEG		0x00000000
#define P1SEG		0x80000000
#define P2SEG		0xa0000000
#define P3SEG		0xc0000000
#define P4SEG		0xe0000000

/* Returns the privileged segment base of a given address */
#define PXSEG(a)	(((unsigned long)(a)) & 0xe0000000)

/* Returns the physical address of a PnSEG (n=1,2) address */
#define PHYSADDR(a)	(((unsigned long)(a)) & 0x1fffffff)

/*
 * Map an address to a certain privileged segment
 */
#define P1SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P1SEG))
#define P2SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P2SEG))
#define P3SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P3SEG))
#define P4SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P4SEG))

/* virt_to_phys will only work when address is in P1 or P2 */
static inline unsigned long virt_to_phys(volatile void *address)
{
	return PHYSADDR(address);
}

static inline void * phys_to_virt(unsigned long address)
{
	return (void *)P1SEGADDR(address);
}

#define cached(addr) ((void *)P1SEGADDR(addr))
#define uncached(addr) ((void *)P2SEGADDR(addr))

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 *
 * This implementation works for memory below 512MiB (flash, etc.) as
 * well as above 3.5GiB (internal peripherals.)
 */
#define MAP_NOCACHE	(0)
#define MAP_WRCOMBINE	(1 << 7)
#define MAP_WRBACK	(MAP_WRCOMBINE | (1 << 9))
#define MAP_WRTHROUGH	(MAP_WRBACK | (1 << 0))

static inline void *
map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
	if (flags == MAP_WRBACK)
		return (void *)P1SEGADDR(paddr);
	else
		return (void *)P2SEGADDR(paddr);
}

#endif /* __ASM_AVR32_ADDRSPACE_H */
