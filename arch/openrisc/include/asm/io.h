/*
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
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

#ifndef __ASM_OPENRISC_IO_H
#define __ASM_OPENRISC_IO_H

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

/*
 * Change virtual addresses to physical addresses
 */
static inline phys_addr_t virt_to_phys(void *vaddr)
{
	return (phys_addr_t)(vaddr);
}


/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the openrisc architecture, we just read/write the
 * memory location directly.
 */
#define readb(addr) (*(volatile unsigned char *) (addr))
#define readw(addr) (*(volatile unsigned short *) (addr))
#define readl(addr) (*(volatile unsigned int *) (addr))
#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl

#define writeb(b, addr) ((*(volatile unsigned char *) (addr)) = (b))
#define writew(b, addr) ((*(volatile unsigned short *) (addr)) = (b))
#define writel(b, addr) ((*(volatile unsigned int *) (addr)) = (b))
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

#define memset_io(a, b, c)	memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)	memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)	memcpy((void *)(a), (b), (c))

/*
 * Again, OpenRISC does not require mem IO specific function.
 */


#define IO_BASE			0x0
#define IO_SPACE_LIMIT		0xffffffff

#define inb(port)		readb((port + IO_BASE))
#define outb(value, port)	writeb((value), (port + IO_BASE))
#define inb_p(port)		inb((port))
#define outb_p(value, port)	outb((value), (port))

/*
 * Convert a physical pointer to a virtual kernel pointer for /dev/mem
 * access
 */
#define xlate_dev_mem_ptr(p)	__va(p)

/*
 * Convert a virtual cached pointer to an uncached pointer
 */
#define xlate_dev_kmem_ptr(p)	p

#define ioread8(addr)		readb(addr)
#define ioread16(addr)		readw(addr)
#define ioread32(addr)		readl(addr)

#define iowrite8(v, addr)	writeb((v), (addr))
#define iowrite16(v, addr)	writew((v), (addr))
#define iowrite32(v, addr)	writel((v), (addr))

#endif
