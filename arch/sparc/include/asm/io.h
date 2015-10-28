/* SPARC I/O definitions
 *
 * (C) Copyright 2007, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SPARC_IO_H
#define _SPARC_IO_H

/* Nothing to sync, total store ordering (TSO)... */
#define sync()

/*
 * Generic virtual read/write.
 */

#ifndef CONFIG_SYS_HAS_NO_CACHE

/* Forces a cache miss on read/load.
 * On some architectures we need to bypass the cache when reading
 * I/O registers so that we are not reading the same status word
 * over and over again resulting in a hang (until an IRQ if lucky)
 */

#define __arch_getb(a)		SPARC_NOCACHE_READ_BYTE((unsigned int)(a))
#define __arch_getw(a)		SPARC_NOCACHE_READ_HWORD((unsigned int)(a))
#define __arch_getl(a)		SPARC_NOCACHE_READ((unsigned int)(a))
#define __arch_getq(a)		SPARC_NOCACHE_READ_DWORD((unsigned int)(a))

#else

#define __arch_getb(a)		(*(volatile unsigned char *)(a))
#define __arch_getw(a)		(*(volatile unsigned short *)(a))
#define __arch_getl(a)		(*(volatile unsigned int *)(a))
#define __arch_getq(a)		(*(volatile unsigned long long *)(a))

#endif /* CONFIG_SYS_HAS_NO_CACHE */

#define __arch_putb(v, a)	(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v, a)	(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v, a)	(*(volatile unsigned int *)(a) = (v))
#define __arch_putq(v, a)	(*(volatile unsigned long long *)(a) = (v))

#define __raw_writeb(v, a)		__arch_putb(v, a)
#define __raw_writew(v, a)		__arch_putw(v, a)
#define __raw_writel(v, a)		__arch_putl(v, a)
#define __raw_writeq(v, a)		__arch_putq(v, a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)
#define __raw_readq(a)			__arch_getq(a)

#define writeb				__raw_writeb
#define writew				__raw_writew
#define writel				__raw_writel
#define writeq				__raw_writeq

#define readb				__raw_readb
#define readw				__raw_readw
#define readl				__raw_readl
#define readq				__raw_readq

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 */

#define MAP_NOCACHE	(0)
#define MAP_WRCOMBINE	(0)
#define MAP_WRBACK	(0)
#define MAP_WRTHROUGH	(0)

static inline void *map_physmem(phys_addr_t paddr, unsigned long len,
				unsigned long flags)
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

#endif
