/* SPARC I/O definitions
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SPARC_IO_H
#define _SPARC_IO_H

/* Nothing to sync, total store ordering (TSO)... */
#define sync()

/* Forces a cache miss on read/load.
 * On some architectures we need to bypass the cache when reading
 * I/O registers so that we are not reading the same status word
 * over and over again resulting in a hang (until an IRQ if lucky)
 *
 */
#ifndef CONFIG_SYS_HAS_NO_CACHE
#define READ_BYTE(var)  SPARC_NOCACHE_READ_BYTE((unsigned int)(var))
#define READ_HWORD(var) SPARC_NOCACHE_READ_HWORD((unsigned int)(var))
#define READ_WORD(var)  SPARC_NOCACHE_READ((unsigned int)(var))
#define READ_DWORD(var) SPARC_NOCACHE_READ_DWORD((unsigned int)(var))
#else
#define READ_BYTE(var)  (var)
#define READ_HWORD(var) (var)
#define READ_WORD(var)  (var)
#define READ_DWORD(var) (var)
#endif

/*
 * Generic virtual read/write.
 */
#define __arch_getb(a)			(READ_BYTE(a))
#define __arch_getw(a)			(READ_HWORD(a))
#define __arch_getl(a)			(READ_WORD(a))
#define __arch_getq(a)			(READ_DWORD(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))

#define __raw_writeb(v,a)		__arch_putb(v,a)
#define __raw_writew(v,a)		__arch_putw(v,a)
#define __raw_writel(v,a)		__arch_putl(v,a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)
#define __raw_readq(a)			__arch_getq(a)

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
