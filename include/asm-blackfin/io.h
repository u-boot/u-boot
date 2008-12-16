/*
 * U-boot - io.h IO routines
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _BLACKFIN_IO_H
#define _BLACKFIN_IO_H

#ifdef __KERNEL__

#include <asm/blackfin.h>

static inline void sync(void)
{
	SSYNC();
}

/* function prototypes for CF support */
extern void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words);
extern void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words);
extern unsigned char cf_inb(volatile unsigned char *addr);
extern void cf_outb(unsigned char val, volatile unsigned char *addr);

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 */
#define MAP_NOCACHE	(0)
#define MAP_WRCOMBINE	(0)
#define MAP_WRBACK	(0)
#define MAP_WRTHROUGH	(0)

static inline void *
map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
	return (void *)paddr;
}

/*
 * Take down a mapping set up by map_physmem().
 */
static inline void unmap_physmem(void *vaddr, unsigned long flags)
{

}

static inline phys_addr_t virt_to_phys(void * vaddr)
{
	return (phys_addr_t)(vaddr);
}

/*
 * These are for ISA/PCI shared memory _only_ and should never be used
 * on any other type of memory, including Zorro memory. They are meant to
 * access the bus in the bus byte order which is little-endian!.
 *
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the m68k architecture, we just read/write the
 * memory location directly.
 */

#ifndef __ASSEMBLY__

static inline unsigned char readb(const volatile void *addr)
{
	unsigned int val;
	int tmp;

	__asm__ __volatile__ ("cli %1;\n\t"
			"NOP; NOP; SSYNC;\n\t"
			"%0 = b [%2] (z);\n\t"
			"sti %1;\n\t"
			: "=d"(val), "=d"(tmp): "a"(addr));

	return (unsigned char) val;
}

static inline unsigned short readw(const volatile void *addr)
{
	unsigned int val;
	int tmp;

	__asm__ __volatile__ ("cli %1;\n\t"
			"NOP; NOP; SSYNC;\n\t"
			"%0 = w [%2] (z);\n\t"
			"sti %1;\n\t"
			: "=d"(val), "=d"(tmp): "a"(addr));

	return (unsigned short) val;
}

static inline unsigned int readl(const volatile void *addr)
{
	unsigned int val;
	int tmp;

	__asm__ __volatile__ ("cli %1;\n\t"
			"NOP; NOP; SSYNC;\n\t"
			"%0 = [%2];\n\t"
			"sti %1;\n\t"
			: "=d"(val), "=d"(tmp): "a"(addr));
	return val;
}

#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl

#endif /*  __ASSEMBLY__ */

#define writeb(b, addr) (void)((*(volatile unsigned char *) (addr)) = (b))
#define writew(b, addr) (void)((*(volatile unsigned short *) (addr)) = (b))
#define writel(b, addr) (void)((*(volatile unsigned int *) (addr)) = (b))
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

#define memset_io(a, b, c)	memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)	memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)	memcpy((void *)(a), (b), (c))

#define inb(addr)		cf_inb((volatile unsigned char *)(addr))
#define outb(x, addr)		cf_outb((unsigned char)(x), (volatile unsigned char *)(addr))

#define insw(port, addr, count)	cf_insw((unsigned short *)addr, (unsigned short *)(port), (count))

#define outsw(port, addr, count)	cf_outsw((unsigned short *)(port), (unsigned short *)addr, (count))

#define IO_SPACE_LIMIT		0xffff

/* Values for nocacheflag and cmode */
#define IOMAP_FULL_CACHING	0
#define IOMAP_NOCACHE_SER	1
#define IOMAP_NOCACHE_NONSER	2
#define IOMAP_WRITETHROUGH	3

extern void *__ioremap(unsigned long physaddr, unsigned long size,
		       int cacheflag);
extern void __iounmap(void *addr, unsigned long size);

extern inline void *ioremap(unsigned long physaddr, unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_NOCACHE_SER);
}
extern inline void *ioremap_nocache(unsigned long physaddr, unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_NOCACHE_SER);
}
extern inline void *ioremap_writethrough(unsigned long physaddr,
					 unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_WRITETHROUGH);
}
extern inline void *ioremap_fullcache(unsigned long physaddr,
				      unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_FULL_CACHING);
}

extern void iounmap(void *addr);

extern void blkfin_inv_cache_all(void);
#define dma_cache_inv(_start, _size)		do { blkfin_inv_cache_all(); } while (0)
#define dma_cache_wback(_start, _size)		do { } while (0)
#define dma_cache_wback_inv(_start, _size)	do { blkfin_inv_cache_all(); } while (0)

#endif
#endif
