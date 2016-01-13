/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARC_IO_H
#define __ASM_ARC_IO_H

#include <linux/types.h>
#include <asm/byteorder.h>

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
	return (void *)((unsigned long)paddr);
}

/*
 * Take down a mapping set up by map_physmem().
 */
static inline void unmap_physmem(void *vaddr, unsigned long flags)
{

}

static inline void sync(void)
{
	/* Not yet implemented */
}

static inline u8 __raw_readb(const volatile void __iomem *addr)
{
	u8 b;

	__asm__ __volatile__("ldb%U1	%0, %1\n"
			     : "=r" (b)
			     : "m" (*(volatile u8 __force *)addr)
			     : "memory");
	return b;
}

static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	u16 s;

	__asm__ __volatile__("ldw%U1	%0, %1\n"
			     : "=r" (s)
			     : "m" (*(volatile u16 __force *)addr)
			     : "memory");
	return s;
}

static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 w;

	__asm__ __volatile__("ld%U1	%0, %1\n"
			     : "=r" (w)
			     : "m" (*(volatile u32 __force *)addr)
			     : "memory");
	return w;
}

#define readb __raw_readb

static inline u16 readw(const volatile void __iomem *addr)
{
	return __le16_to_cpu(__raw_readw(addr));
}

static inline u32 readl(const volatile void __iomem *addr)
{
	return __le32_to_cpu(__raw_readl(addr));
}

static inline void __raw_writeb(u8 b, volatile void __iomem *addr)
{
	__asm__ __volatile__("stb%U1	%0, %1\n"
			     :
			     : "r" (b), "m" (*(volatile u8 __force *)addr)
			     : "memory");
}

static inline void __raw_writew(u16 s, volatile void __iomem *addr)
{
	__asm__ __volatile__("stw%U1	%0, %1\n"
			     :
			     : "r" (s), "m" (*(volatile u16 __force *)addr)
			     : "memory");
}

static inline void __raw_writel(u32 w, volatile void __iomem *addr)
{
	__asm__ __volatile__("st%U1	%0, %1\n"
			     :
			     : "r" (w), "m" (*(volatile u32 __force *)addr)
			     : "memory");
}

#define writeb __raw_writeb
#define writew(b, addr) __raw_writew(__cpu_to_le16(b), addr)
#define writel(b, addr) __raw_writel(__cpu_to_le32(b), addr)

static inline int __raw_readsb(unsigned int addr, void *data, int bytelen)
{
	__asm__ __volatile__ ("1:ld.di	r8, [r0]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "stb.ab	r8, [r1, 1]\n"
			      :
			      : "r" (addr), "r" (data), "r" (bytelen)
			      : "r8");
	return bytelen;
}

static inline int __raw_readsw(unsigned int addr, void *data, int wordlen)
{
	__asm__ __volatile__ ("1:ld.di	r8, [r0]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "stw.ab	r8, [r1, 2]\n"
			      :
			      : "r" (addr), "r" (data), "r" (wordlen)
			      : "r8");
	return wordlen;
}

static inline int __raw_readsl(unsigned int addr, void *data, int longlen)
{
	__asm__ __volatile__ ("1:ld.di	r8, [r0]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.ab	r8, [r1, 4]\n"
			      :
			      : "r" (addr), "r" (data), "r" (longlen)
			      : "r8");
	return longlen;
}

static inline int __raw_writesb(unsigned int addr, void *data, int bytelen)
{
	__asm__ __volatile__ ("1:ldb.ab	r8, [r1, 1]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.di	r8, [r0, 0]\n"
			      :
			      : "r" (addr), "r" (data), "r" (bytelen)
			      : "r8");
	return bytelen;
}

static inline int __raw_writesw(unsigned int addr, void *data, int wordlen)
{
	__asm__ __volatile__ ("1:ldw.ab	r8, [r1, 2]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.ab.di	r8, [r0, 0]\n"
			      :
			      : "r" (addr), "r" (data), "r" (wordlen)
			      : "r8");
	return wordlen;
}

static inline int __raw_writesl(unsigned int addr, void *data, int longlen)
{
	__asm__ __volatile__ ("1:ld.ab	r8, [r1, 4]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.ab.di	r8, [r0, 0]\n"
			      :
			      : "r" (addr), "r" (data), "r" (longlen)
			      : "r8");
	return longlen;
}

#define out_arch(type, endian, a, v)	__raw_write##type(cpu_to_##endian(v), a)
#define in_arch(type, endian, a)	endian##_to_cpu(__raw_read##type(a))

#define out_le32(a, v)	out_arch(l, le32, a, v)
#define out_le16(a, v)	out_arch(w, le16, a, v)

#define in_le32(a)	in_arch(l, le32, a)
#define in_le16(a)	in_arch(w, le16, a)

#define out_be32(a, v)	out_arch(l, be32, a, v)
#define out_be16(a, v)	out_arch(w, be16, a, v)

#define in_be32(a)	in_arch(l, be32, a)
#define in_be16(a)	in_arch(w, be16, a)

#define out_8(a, v)	__raw_writeb(v, a)
#define in_8(a)		__raw_readb(a)

/*
 * Clear and set bits in one shot. These macros can be used to clear and
 * set multiple bits in a register using a single call. These macros can
 * also be used to set a multiple-bit bit pattern using a mask, by
 * specifying the mask in the 'clear' parameter and the new bit pattern
 * in the 'set' parameter.
 */

#define clrbits(type, addr, clear) \
	out_##type((addr), in_##type(addr) & ~(clear))

#define setbits(type, addr, set) \
	out_##type((addr), in_##type(addr) | (set))

#define clrsetbits(type, addr, clear, set) \
	out_##type((addr), (in_##type(addr) & ~(clear)) | (set))

#define clrbits_be32(addr, clear) clrbits(be32, addr, clear)
#define setbits_be32(addr, set) setbits(be32, addr, set)
#define clrsetbits_be32(addr, clear, set) clrsetbits(be32, addr, clear, set)

#define clrbits_le32(addr, clear) clrbits(le32, addr, clear)
#define setbits_le32(addr, set) setbits(le32, addr, set)
#define clrsetbits_le32(addr, clear, set) clrsetbits(le32, addr, clear, set)

#define clrbits_be16(addr, clear) clrbits(be16, addr, clear)
#define setbits_be16(addr, set) setbits(be16, addr, set)
#define clrsetbits_be16(addr, clear, set) clrsetbits(be16, addr, clear, set)

#define clrbits_le16(addr, clear) clrbits(le16, addr, clear)
#define setbits_le16(addr, set) setbits(le16, addr, set)
#define clrsetbits_le16(addr, clear, set) clrsetbits(le16, addr, clear, set)

#define clrbits_8(addr, clear) clrbits(8, addr, clear)
#define setbits_8(addr, set) setbits(8, addr, set)
#define clrsetbits_8(addr, clear, set) clrsetbits(8, addr, clear, set)

#endif	/* __ASM_ARC_IO_H */
