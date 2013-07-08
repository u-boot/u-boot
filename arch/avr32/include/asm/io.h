/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_IO_H
#define __ASM_AVR32_IO_H

#include <asm/types.h>

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

#include <asm/arch/addrspace.h>
/* Provides virt_to_phys, phys_to_virt, cached, uncached, map_physmem */

#endif /* __KERNEL__ */

static inline void sync(void)
{
}

/*
 * Take down a mapping set up by map_physmem().
 */
static inline void unmap_physmem(void *vaddr, unsigned long len)
{

}

#endif /* __ASM_AVR32_IO_H */
