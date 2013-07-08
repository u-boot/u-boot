/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_NIOS2_IO_H_
#define __ASM_NIOS2_IO_H_

static inline void sync(void)
{
	__asm__ __volatile__ ("sync" : : : "memory");
}

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

extern unsigned char inb (unsigned char *port);
extern unsigned short inw (unsigned short *port);
extern unsigned inl (unsigned port);

#define __raw_writeb(v,a)       (*(volatile unsigned char  *)(a) = (v))
#define __raw_writew(v,a)       (*(volatile unsigned short *)(a) = (v))
#define __raw_writel(v,a)       (*(volatile unsigned int   *)(a) = (v))

#define __raw_readb(a)          (*(volatile unsigned char  *)(a))
#define __raw_readw(a)          (*(volatile unsigned short *)(a))
#define __raw_readl(a)          (*(volatile unsigned int   *)(a))

#define readb(addr)\
	({unsigned char val;\
	 asm volatile( "ldbio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define readw(addr)\
	({unsigned short val;\
	 asm volatile( "ldhio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define readl(addr)\
	({unsigned long val;\
	 asm volatile( "ldwio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})

#define writeb(val,addr)\
	asm volatile ("stbio %0, 0(%1)" : : "r" (val), "r" (addr))
#define writew(val,addr)\
	asm volatile ("sthio %0, 0(%1)" : : "r" (val), "r" (addr))
#define writel(val,addr)\
	asm volatile ("stwio %0, 0(%1)" : : "r" (val), "r" (addr))

#define inb(addr)	readb(addr)
#define inw(addr)	readw(addr)
#define inl(addr)	readl(addr)
#define outb(val, addr)	writeb(val,addr)
#define outw(val, addr)	writew(val,addr)
#define outl(val, addr)	writel(val,addr)

static inline void insb (unsigned long port, void *dst, unsigned long count)
{
	unsigned char *p = dst;
	while (count--) *p++ = inb (port);
}
static inline void insw (unsigned long port, void *dst, unsigned long count)
{
	unsigned short *p = dst;
	while (count--) *p++ = inw (port);
}
static inline void insl (unsigned long port, void *dst, unsigned long count)
{
	unsigned long *p = dst;
	while (count--) *p++ = inl (port);
}

static inline void outsb (unsigned long port, const void *src, unsigned long count)
{
	const unsigned char *p = src;
	while (count--) outb (*p++, port);
}

static inline void outsw (unsigned long port, const void *src, unsigned long count)
{
	const unsigned short *p = src;
	while (count--) outw (*p++, port);
}
static inline void outsl (unsigned long port, const void *src, unsigned long count)
{
	const unsigned long *p = src;
	while (count--) outl (*p++, port);
}

#endif /* __ASM_NIOS2_IO_H_ */
