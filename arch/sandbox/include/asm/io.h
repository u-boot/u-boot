/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SANDBOX_ASM_IO_H
#define __SANDBOX_ASM_IO_H

void *phys_to_virt(phys_addr_t paddr);
#define phys_to_virt phys_to_virt

phys_addr_t virt_to_phys(void *vaddr);
#define virt_to_phys virt_to_phys

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags);
#define map_physmem map_physmem

/*
 * Take down a mapping set up by map_physmem().
 */
void unmap_physmem(const void *vaddr, unsigned long flags);
#define unmap_physmem unmap_physmem

#include <asm-generic/io.h>

/* For sandbox, we want addresses to point into our RAM buffer */
static inline void *map_sysmem(phys_addr_t paddr, unsigned long len)
{
	return map_physmem(paddr, len, MAP_WRBACK);
}

/* Remove a previous mapping */
static inline void unmap_sysmem(const void *vaddr)
{
	unmap_physmem(vaddr, MAP_WRBACK);
}

/* Map from a pointer to our RAM buffer */
phys_addr_t map_to_sysmem(const void *ptr);

/* Define nops for sandbox I/O access */
#define readb(addr) ((void)addr, 0)
#define readw(addr) ((void)addr, 0)
#define readl(addr) ((void)addr, 0)
#define writeb(v, addr) ((void)addr)
#define writew(v, addr) ((void)addr)
#define writel(v, addr) ((void)addr)

/* I/O access functions */
int inl(unsigned int addr);
int inw(unsigned int addr);
int inb(unsigned int addr);

void outl(unsigned int value, unsigned int addr);
void outw(unsigned int value, unsigned int addr);
void outb(unsigned int value, unsigned int addr);

static inline void _insw(volatile u16 *port, void *buf, int ns)
{
}

static inline void _outsw(volatile u16 *port, const void *buf, int ns)
{
}

#define insw(port, buf, ns)		_insw((u16 *)port, buf, ns)
#define outsw(port, buf, ns)		_outsw((u16 *)port, buf, ns)

/* For systemace.c */
#define out16(addr, val)
#define in16(addr)		0

#include <iotrace.h>
#include <asm/types.h>

#endif
