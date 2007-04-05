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

#include <linux/config.h>

/* function prototypes for CF support */
extern void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words);
extern void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words);
extern unsigned char cf_inb(volatile unsigned char *addr);
extern void cf_outb(unsigned char val, volatile unsigned char *addr);

static inline void sync(void)
{
	__builtin_bfin_ssync();
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

#define readb(addr)		({ unsigned char __v = (*(volatile unsigned char *) (addr));asm("ssync;"); __v; })
#define readw(addr)		({ unsigned short __v = (*(volatile unsigned short *) (addr)); asm("ssync;");__v; })
#define readl(addr)		({ unsigned int __v = (*(volatile unsigned int *) (addr));asm("ssync;"); __v; })

#define writeb(b,addr)		{((*(volatile unsigned char *) (addr)) = (b)); asm("ssync;");}
#define writew(b,addr)		{((*(volatile unsigned short *) (addr)) = (b)); asm("ssync;");}
#define writel(b,addr)		{((*(volatile unsigned int *) (addr)) = (b)); asm("ssync;");}

#define memset_io(a,b,c)	memset((void *)(a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),(void *)(b),(c))
#define memcpy_toio(a,b,c)	memcpy((void *)(a),(b),(c))

#define inb_p(addr)		readb((addr) + BF533_PCIIO_BASE)
#define inb(addr)		cf_inb((volatile unsigned char*)(addr))

#define outb(x,addr)		cf_outb((unsigned char)(x), (volatile unsigned char*)(addr))
#define outb_p(x,addr)		outb(x, (addr) + BF533_PCIIO_BASE)

#define inw(addr)		readw((addr) + BF533_PCIIO_BASE)
#define inl(addr)		readl((addr) + BF533_PCIIO_BASE)

#define outw(x,addr)		writew(x, (addr) + BF533_PCIIO_BASE)
#define outl(x,addr)		writel(x, (addr) + BF533_PCIIO_BASE)

#define insb(port, addr, count)	memcpy((void*)addr, (void*)(BF533_PCIIO_BASE + port), count)
#define insw(port, addr, count)	cf_insw((unsigned short*)addr, (unsigned short*)(port), (count))
#define insl(port, addr, count)	memcpy((void*)addr, (void*)(BF533_PCIIO_BASE + port), (4*count))

#define outsb(port,addr,count)	memcpy((void*)(BF533_PCIIO_BASE + port), (void*)addr, count)
#define outsw(port,addr,count)	cf_outsw((unsigned short*)(port), (unsigned short*)addr, (count))
#define outsl(port,addr,count)	memcpy((void*)(BF533_PCIIO_BASE + port), (void*)addr, (4*count))

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
#define dma_cache_inv(_start,_size)		do { blkfin_inv_cache_all();} while (0)
#define dma_cache_wback(_start,_size)		do { } while (0)
#define dma_cache_wback_inv(_start,_size)	do { blkfin_inv_cache_all();} while (0)

#endif
#endif
