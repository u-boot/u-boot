/*
 * U-Boot - io.h IO routines
 *
 * Copyright 2004-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef _BLACKFIN_IO_H
#define _BLACKFIN_IO_H

#ifdef __KERNEL__

#include <linux/compiler.h>
#include <asm/blackfin.h>

static inline void sync(void)
{
	SSYNC();
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

/*
 * These are for ISA/PCI shared memory _only_ and should never be used
 * on any other type of memory, including Zorro memory. They are meant to
 * access the bus in the bus byte order which is little-endian!.
 *
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the bfin architecture, we just read/write the
 * memory location directly.
 */
#ifndef __ASSEMBLY__

static inline unsigned char readb(const volatile void __iomem *addr)
{
	unsigned int val;
	int tmp;

	__asm__ __volatile__ (
		"cli %1;"
		"NOP; NOP; SSYNC;"
		"%0 = b [%2] (z);"
		"sti %1;"
		: "=d"(val), "=d"(tmp)
		: "a"(addr)
	);

	return (unsigned char) val;
}

static inline unsigned short readw(const volatile void __iomem *addr)
{
	unsigned int val;
	int tmp;

	__asm__ __volatile__ (
		"cli %1;"
		"NOP; NOP; SSYNC;"
		"%0 = w [%2] (z);"
		"sti %1;"
		: "=d"(val), "=d"(tmp)
		: "a"(addr)
	);

	return (unsigned short) val;
}

static inline unsigned int readl(const volatile void __iomem *addr)
{
	unsigned int val;
	int tmp;

	__asm__ __volatile__ (
		"cli %1;"
		"NOP; NOP; SSYNC;"
		"%0 = [%2];"
		"sti %1;"
		: "=d"(val), "=d"(tmp)
		: "a"(addr)
	);

	return val;
}

#endif /*  __ASSEMBLY__ */

#define writeb(b, addr) (void)((*(volatile unsigned char *) (addr)) = (b))
#define writew(b, addr) (void)((*(volatile unsigned short *) (addr)) = (b))
#define writel(b, addr) (void)((*(volatile unsigned int *) (addr)) = (b))

#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel
#define memset_io(a, b, c)	memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)	memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)	memcpy((void *)(a), (b), (c))

/* Convert "I/O port addresses" to actual addresses.  i.e. ugly casts. */
#define __io(port) ((void *)(unsigned long)(port))

#define inb(port)    readb(__io(port))
#define inw(port)    readw(__io(port))
#define inl(port)    readl(__io(port))
#define in_le32(port) inl(port)
#define outb(x, port) writeb(x, __io(port))
#define outw(x, port) writew(x, __io(port))
#define outl(x, port) writel(x, __io(port))
#define out_le32(x, port) outl(x, port)

#define inb_p(port)    inb(__io(port))
#define inw_p(port)    inw(__io(port))
#define inl_p(port)    inl(__io(port))
#define outb_p(x, port) outb(x, __io(port))
#define outw_p(x, port) outw(x, __io(port))
#define outl_p(x, port) outl(x, __io(port))

#define ioread8_rep(a, d, c)	readsb(a, d, c)
#define ioread16_rep(a, d, c)	readsw(a, d, c)
#define ioread32_rep(a, d, c)	readsl(a, d, c)
#define iowrite8_rep(a, s, c)	writesb(a, s, c)
#define iowrite16_rep(a, s, c)	writesw(a, s, c)
#define iowrite32_rep(a, s, c)	writesl(a, s, c)

#define ioread8(x)			readb(x)
#define ioread16(x)			readw(x)
#define ioread32(x)			readl(x)
#define iowrite8(val, x)		writeb(val, x)
#define iowrite16(val, x)		writew(val, x)
#define iowrite32(val, x)		writel(val, x)

#define mmiowb() wmb()

#ifndef __ASSEMBLY__

extern void outsb(unsigned long port, const void *addr, unsigned long count);
extern void outsw(unsigned long port, const void *addr, unsigned long count);
extern void outsw_8(unsigned long port, const void *addr, unsigned long count);
extern void outsl(unsigned long port, const void *addr, unsigned long count);

extern void insb(unsigned long port, void *addr, unsigned long count);
extern void insw(unsigned long port, void *addr, unsigned long count);
extern void insw_8(unsigned long port, void *addr, unsigned long count);
extern void insl(unsigned long port, void *addr, unsigned long count);
extern void insl_16(unsigned long port, void *addr, unsigned long count);

static inline void readsl(const void __iomem *addr, void *buf, int len)
{
	insl((unsigned long)addr, buf, len);
}

static inline void readsw(const void __iomem *addr, void *buf, int len)
{
	insw((unsigned long)addr, buf, len);
}

static inline void readsb(const void __iomem *addr, void *buf, int len)
{
	insb((unsigned long)addr, buf, len);
}

static inline void writesl(const void __iomem *addr, const void *buf, int len)
{
	outsl((unsigned long)addr, buf, len);
}

static inline void writesw(const void __iomem *addr, const void *buf, int len)
{
	outsw((unsigned long)addr, buf, len);
}

static inline void writesb(const void __iomem *addr, const void *buf, int len)
{
	outsb((unsigned long)addr, buf, len);
}

#if defined(CONFIG_STAMP_CF) || defined(CONFIG_BFIN_IDE)
/* This hack for CF/IDE needs to be addressed at some point */
extern void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words);
extern void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words);
extern unsigned char cf_inb(volatile unsigned char *addr);
extern void cf_outb(unsigned char val, volatile unsigned char *addr);
#undef inb
#undef outb
#undef insw
#undef outsw
#define inb(addr) cf_inb((void *)(addr))
#define outb(x, addr) cf_outb((unsigned char)(x), (void *)(addr))
#define insw(port, addr, cnt) cf_insw((void *)(addr), (void *)(port), cnt)
#define outsw(port, addr, cnt) cf_outsw((void *)(port), (void *)(addr), cnt)
#endif

#endif

#endif				/* __KERNEL__ */

#endif
