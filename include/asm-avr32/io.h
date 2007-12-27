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
#ifndef __ASM_AVR32_IO_H
#define __ASM_AVR32_IO_H

#ifdef __KERNEL__

/*
 * Generic IO read/write.  These perform native-endian accesses.  Note
 * that some architectures will want to re-define __raw_{read,write}w.
 */
extern void __raw_writesb(unsigned int addr, const void *data, int bytelen);
extern void __raw_writesw(unsigned int addr, const void *data, int wordlen);
extern void __raw_writesl(unsigned int addr, const void *data, int longlen);

extern void __raw_readsb(unsigned int addr, void *data, int bytelen);
extern void __raw_readsw(unsigned int addr, void *data, int wordlen);
extern void __raw_readsl(unsigned int addr, void *data, int longlen);

#define __raw_writeb(v,a)       (*(volatile unsigned char  *)(a) = (v))
#define __raw_writew(v,a)       (*(volatile unsigned short *)(a) = (v))
#define __raw_writel(v,a)       (*(volatile unsigned int   *)(a) = (v))

#define __raw_readb(a)          (*(volatile unsigned char  *)(a))
#define __raw_readw(a)          (*(volatile unsigned short *)(a))
#define __raw_readl(a)          (*(volatile unsigned int   *)(a))

/* As long as I/O is only performed in P4 (or possibly P3), we're safe */
#define writeb(v,a)		__raw_writeb(v,a)
#define writew(v,a)		__raw_writew(v,a)
#define writel(v,a)		__raw_writel(v,a)

#define readb(a)		__raw_readb(a)
#define readw(a)		__raw_readw(a)
#define readl(a)		__raw_readl(a)

/*
 * Bad read/write accesses...
 */
extern void __readwrite_bug(const char *fn);

#define IO_SPACE_LIMIT	0xffffffff

/*
 * All I/O is memory mapped, so these macros doesn't make very much sense
 */
#define outb(v,p)		__raw_writeb(v, p)
#define outw(v,p)		__raw_writew(cpu_to_le16(v),p)
#define outl(v,p)		__raw_writel(cpu_to_le32(v),p)

#define inb(p)	({ unsigned int __v = __raw_readb(p); __v; })
#define inw(p)	({ unsigned int __v = __le16_to_cpu(__raw_readw(p)); __v; })
#define inl(p)	({ unsigned int __v = __le32_to_cpu(__raw_readl(p)); __v; })

#include <asm/addrspace.h>

/* virt_to_phys will only work when address is in P1 or P2 */
static __inline__ unsigned long virt_to_phys(volatile void *address)
{
	return PHYSADDR(address);
}

static __inline__ void * phys_to_virt(unsigned long address)
{
	return (void *)P1SEGADDR(address);
}

#define cached(addr) ((void *)P1SEGADDR(addr))
#define uncached(addr) ((void *)P2SEGADDR(addr))

#endif /* __KERNEL__ */

static inline void sync(void)
{
}

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 *
 * This implementation works for memory below 512MiB (flash, etc.) as
 * well as above 3.5GiB (internal peripherals.)
 */
typedef unsigned long phys_addr_t;

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

/*
 * Take down a mapping set up by map_physmem().
 */
static inline void unmap_physmem(void *vaddr, unsigned long len)
{

}

#endif /* __ASM_AVR32_IO_H */
