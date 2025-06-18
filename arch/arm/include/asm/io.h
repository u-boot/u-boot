/*
 * I/O device access primitives. Based on early versions from the Linux kernel.
 *
 *  Copyright (C) 1996-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/byteorder.h>
#include <asm/memory.h>
#include <asm/barriers.h>

static inline void sync(void)
{
}

#ifdef CONFIG_ARM64
#define __W	"w"
#else
#define __W
#endif

#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
#define __R "l"
#define __RM "=l"
#else
#define __R "r"
#define __RM "=r"
#endif

#ifdef CONFIG_KVM_VIRT_INS
/*
 * The __raw_writeX/__raw_readX below should be converted to static inline
 * functions. However doing so produces a lot of compilation warnings when
 * called with a raw address. Convert these once the callers have been fixed.
 */
#define __raw_writeb(val, addr)			\
	do {					\
		asm volatile("strb %" __W "0, [%1]"	\
		:				\
		: __R ((u8)(val)), __R (addr));	\
	} while (0)

#define __raw_readb(addr)				\
	({						\
		u32 __val;				\
		asm volatile("ldrb %" __W "0, [%1]"		\
		: __RM (__val)				\
		: __R (addr));				\
		__val;					\
	})

#define __raw_writew(val, addr)			\
	do {					\
		asm volatile("strh %" __W "0, [%1]"	\
		:					\
		: __R ((u16)(val)), __R (addr));	\
	} while (0)

#define __raw_readw(addr)				\
	({						\
		u32 __val;				\
		asm volatile("ldrh %" __W "0, [%1]"		\
		: __RM (__val)				\
		: __R (addr));				\
	__val;						\
    })

#define __raw_writel(val, addr)				\
	do {						\
		asm volatile("str %" __W "0, [%1]"		\
		:					\
		: __R ((u32)(val)), __R (addr));	\
	} while (0)

#define __raw_readl(addr)				\
	({						\
		u32 __val;				\
		asm volatile("ldr %" __W "0, [%1]"		\
		: __RM (__val)				\
		: __R (addr));				\
		__val;					\
	})

#define __raw_writeq(val, addr)				\
	do {						\
		asm volatile("str %0, [%1]"		\
		:					\
		: __R ((u64)(val)), __R (addr));	\
	} while (0)

#define __raw_readq(addr)				\
	({						\
		u64 __val;				\
		asm volatile("ldr %0, [%1]"		\
		: __RM (__val)				\
		: __R (addr));				\
		__val;					\
	    })
#else
/* Generic virtual read/write. */
#define __raw_readb(a)			(*(volatile unsigned char *)(a))
#define __raw_readw(a)			(*(volatile unsigned short *)(a))
#define __raw_readl(a)			(*(volatile unsigned int *)(a))
#define __raw_readq(a)			(*(volatile unsigned long long *)(a))

#define __raw_writeb(v, a)		(*(volatile unsigned char *)(a) = (v))
#define __raw_writew(v, a)		(*(volatile unsigned short *)(a) = (v))
#define __raw_writel(v, a)		(*(volatile unsigned int *)(a) = (v))
#define __raw_writeq(v, a)		(*(volatile unsigned long long *)(a) = (v))
#endif

static inline void __raw_writesb(unsigned long addr, const void *data,
				 int bytelen)
{
	uint8_t *buf = (uint8_t *)data;
	while(bytelen--)
		__raw_writeb(*buf++, addr);
}

static inline void __raw_writesw(unsigned long addr, const void *data,
				 int wordlen)
{
	uint16_t *buf = (uint16_t *)data;
	while(wordlen--)
		__raw_writew(*buf++, addr);
}

static inline void __raw_writesl(unsigned long addr, const void *data,
				 int longlen)
{
	uint32_t *buf = (uint32_t *)data;
	while(longlen--)
		__raw_writel(*buf++, addr);
}

static inline void __raw_readsb(unsigned long addr, void *data, int bytelen)
{
	uint8_t *buf = (uint8_t *)data;
	while(bytelen--)
		*buf++ = __raw_readb(addr);
}

static inline void __raw_readsw(unsigned long addr, void *data, int wordlen)
{
	uint16_t *buf = (uint16_t *)data;
	while(wordlen--)
		*buf++ = __raw_readw(addr);
}

static inline void __raw_readsl(unsigned long addr, void *data, int longlen)
{
	uint32_t *buf = (uint32_t *)data;
	while(longlen--)
		*buf++ = __raw_readl(addr);
}

/*
 * TODO: The kernel offers some more advanced versions of barriers, it might
 * have some advantages to use them instead of the simple one here.
 */
#define mb()		dsb()
#define rmb()		dsb()
#define wmb()		dsb()
#define __iormb()	dmb()
#define __iowmb()	dmb()

#define smp_processor_id()	0

#define writeb(v, c)	({ u8  __v = v; __iowmb(); writeb_relaxed(__v, c); __v; })
#define writew(v, c)	({ u16 __v = v; __iowmb(); writew_relaxed(__v, c); __v; })
#define writel(v, c)	({ u32 __v = v; __iowmb(); writel_relaxed(__v, c); __v; })
#define writeq(v, c)	({ u64 __v = v; __iowmb(); writeq_relaxed(__v, c); __v; })

#define readb(c)	({ u8  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)	({ u16 __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)	({ u32 __v = readl_relaxed(c); __iormb(); __v; })
#define readq(c)	({ u64 __v = readq_relaxed(c); __iormb(); __v; })

/*
 * Relaxed I/O memory access primitives. These follow the Device memory
 * ordering rules but do not guarantee any ordering relative to Normal memory
 * accesses.
 */
#define readb_relaxed(c)	({ u8  __r = __raw_readb(c); __r; })
#define readw_relaxed(c)	({ u16 __r = le16_to_cpu((__force __le16) \
						__raw_readw(c)); __r; })
#define readl_relaxed(c)	({ u32 __r = le32_to_cpu((__force __le32) \
						__raw_readl(c)); __r; })
#define readq_relaxed(c)	({ u64 __r = le64_to_cpu((__force __le64) \
						__raw_readq(c)); __r; })

#define writeb_relaxed(v, c)	__raw_writeb((v), (c))
#define writew_relaxed(v, c)	__raw_writew((__force u16)cpu_to_le16(v), (c))
#define writel_relaxed(v, c)	__raw_writel((__force u32)cpu_to_le32(v), (c))
#define writeq_relaxed(v, c)	__raw_writeq((__force u64)cpu_to_le64(v), (c))

/*
 * The compiler seems to be incapable of optimising constants
 * properly.  Spell it out to the compiler in some cases.
 * These are only valid for small values of "off" (< 1<<12)
 */
#define __raw_base_writeb(val,base,off)	__arch_base_putb(val,base,off)
#define __raw_base_writew(val,base,off)	__arch_base_putw(val,base,off)
#define __raw_base_writel(val,base,off)	__arch_base_putl(val,base,off)

#define __raw_base_readb(base,off)	__arch_base_getb(base,off)
#define __raw_base_readw(base,off)	__arch_base_getw(base,off)
#define __raw_base_readl(base,off)	__arch_base_getl(base,off)

/*
 * Clear and set bits in one shot. These macros can be used to clear and
 * set multiple bits in a register using a single call. These macros can
 * also be used to set a multiple-bit bit pattern using a mask, by
 * specifying the mask in the 'clear' parameter and the new bit pattern
 * in the 'set' parameter.
 */

#define out_arch(type,endian,a,v)	__raw_write##type(cpu_to_##endian(v),a)
#define in_arch(type,endian,a)		endian##_to_cpu(__raw_read##type(a))

#define out_le64(a,v)	out_arch(q,le64,a,v)
#define out_le32(a,v)	out_arch(l,le32,a,v)
#define out_le16(a,v)	out_arch(w,le16,a,v)

#define in_le64(a)	in_arch(q,le64,a)
#define in_le32(a)	in_arch(l,le32,a)
#define in_le16(a)	in_arch(w,le16,a)

#define out_be64(a,v)	out_arch(l,be64,a,v)
#define out_be32(a,v)	out_arch(l,be32,a,v)
#define out_be16(a,v)	out_arch(w,be16,a,v)

#define in_be64(a)	in_arch(l,be64,a)
#define in_be32(a)	in_arch(l,be32,a)
#define in_be16(a)	in_arch(w,be16,a)

#define out_64(a,v)	__raw_writeq(v,a)
#define out_32(a,v)	__raw_writel(v,a)
#define out_16(a,v)	__raw_writew(v,a)
#define out_8(a,v)	__raw_writeb(v,a)

#define in_64(a)	__raw_readq(a)
#define in_32(a)	__raw_readl(a)
#define in_16(a)	__raw_readw(a)
#define in_8(a)		__raw_readb(a)

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

#define clrbits_32(addr, clear) clrbits(32, addr, clear)
#define setbits_32(addr, set) setbits(32, addr, set)
#define clrsetbits_32(addr, clear, set) clrsetbits(32, addr, clear, set)

#define clrbits_be16(addr, clear) clrbits(be16, addr, clear)
#define setbits_be16(addr, set) setbits(be16, addr, set)
#define clrsetbits_be16(addr, clear, set) clrsetbits(be16, addr, clear, set)

#define clrbits_le16(addr, clear) clrbits(le16, addr, clear)
#define setbits_le16(addr, set) setbits(le16, addr, set)
#define clrsetbits_le16(addr, clear, set) clrsetbits(le16, addr, clear, set)

#define clrbits_16(addr, clear) clrbits(16, addr, clear)
#define setbits_16(addr, set) setbits(16, addr, set)
#define clrsetbits_16(addr, clear, set) clrsetbits(16, addr, clear, set)

#define clrbits_8(addr, clear) clrbits(8, addr, clear)
#define setbits_8(addr, set) setbits(8, addr, set)
#define clrsetbits_8(addr, clear, set) clrsetbits(8, addr, clear, set)

#define clrbits_be64(addr, clear) clrbits(be64, addr, clear)
#define setbits_be64(addr, set) setbits(be64, addr, set)
#define clrsetbits_be64(addr, clear, set) clrsetbits(be64, addr, clear, set)

#define clrbits_le64(addr, clear) clrbits(le64, addr, clear)
#define setbits_le64(addr, set) setbits(le64, addr, set)
#define clrsetbits_le64(addr, clear, set) clrsetbits(le64, addr, clear, set)

#define clrbits_64(addr, clear) clrbits(64, addr, clear)
#define setbits_64(addr, set) setbits(64, addr, set)
#define clrsetbits_64(addr, clear, set) clrsetbits(64, addr, clear, set)

/*
 *  IO port access primitives
 *  -------------------------
 *
 * The ARM doesn't have special IO access instructions; all IO is memory
 * mapped.  Note that these are defined to perform little endian accesses
 * only.  Their primary purpose is to access PCI and ISA peripherals.
 *
 * Note that for a big endian machine, this implies that the following
 * big endian mode connectivity is in place, as described by numerous
 * ARM documents:
 *
 *    PCI:  D0-D7   D8-D15 D16-D23 D24-D31
 *    ARM: D24-D31 D16-D23  D8-D15  D0-D7
 *
 * The machine specific io.h include defines __io to translate an "IO"
 * address to a memory address.
 *
 * Note that we prevent GCC re-ordering or caching values in expressions
 * by introducing sequence points into the in*() definitions.  Note that
 * __raw_* do not guarantee this behaviour.
 *
 * The {in,out}[bwl] macros are for emulating x86-style PCI/ISA IO space.
 */
#ifdef __io
#define outb(v,p)			__raw_writeb(v,__io(p))
#define outw(v,p)			__raw_writew(cpu_to_le16(v),__io(p))
#define outl(v,p)			__raw_writel(cpu_to_le32(v),__io(p))

#define inb(p)	({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)	({ unsigned int __v = le16_to_cpu(__raw_readw(__io(p))); __v; })
#define inl(p)	({ unsigned int __v = le32_to_cpu(__raw_readl(__io(p))); __v; })

#define outsb(p,d,l)			__raw_writesb(__io(p),d,l)
#define outsw(p,d,l)			__raw_writesw(__io(p),d,l)
#define outsl(p,d,l)			__raw_writesl(__io(p),d,l)

#define insb(p,d,l)			__raw_readsb(__io(p),d,l)
#define insw(p,d,l)			__raw_readsw(__io(p),d,l)
#define insl(p,d,l)			__raw_readsl(__io(p),d,l)
#endif

#define outb_p(val,port)		outb((val),(port))
#define outw_p(val,port)		outw((val),(port))
#define outl_p(val,port)		outl((val),(port))
#define inb_p(port)			inb((port))
#define inw_p(port)			inw((port))
#define inl_p(port)			inl((port))

#define outsb_p(port,from,len)		outsb(port,from,len)
#define outsw_p(port,from,len)		outsw(port,from,len)
#define outsl_p(port,from,len)		outsl(port,from,len)
#define insb_p(port,to,len)		insb(port,to,len)
#define insw_p(port,to,len)		insw(port,to,len)
#define insl_p(port,to,len)		insl(port,to,len)

#define writesl(a, d, s)	__raw_writesl((unsigned long)a, d, s)
#define readsl(a, d, s)		__raw_readsl((unsigned long)a, d, s)
#define writesw(a, d, s)	__raw_writesw((unsigned long)a, d, s)
#define readsw(a, d, s)		__raw_readsw((unsigned long)a, d, s)
#define writesb(a, d, s)	__raw_writesb((unsigned long)a, d, s)
#define readsb(a, d, s)		__raw_readsb((unsigned long)a, d, s)

/*
 * String version of IO memory access ops:
 */
extern void _memcpy_fromio(void *, unsigned long, size_t);
extern void _memcpy_toio(unsigned long, const void *, size_t);
extern void _memset_io(unsigned long, int, size_t);

/* Optimized copy functions to read from/write to IO sapce */
#ifdef CONFIG_ARM64
#include <cpu_func.h>
/*
 * Copy data from IO memory space to "real" memory space.
 */
static inline
void __memcpy_fromio(void *to, const volatile void __iomem *from, size_t count)
{
	while (count && !IS_ALIGNED((unsigned long)from, 8)) {
		*(u8 *)to = __raw_readb(from);
		from++;
		to++;
		count--;
	}

	if (mmu_status()) {
		while (count >= 8) {
			*(u64 *)to = __raw_readq(from);
			from += 8;
			to += 8;
			count -= 8;
		}
	}

	while (count) {
		*(u8 *)to = __raw_readb(from);
		from++;
		to++;
		count--;
	}
}

/*
 * Copy data from "real" memory space to IO memory space.
 */
static inline
void __memcpy_toio(volatile void __iomem *to, const void *from, size_t count)
{
	while (count && !IS_ALIGNED((unsigned long)to, 8)) {
		__raw_writeb(*(u8 *)from, to);
		from++;
		to++;
		count--;
	}

	if (mmu_status()) {
		while (count >= 8) {
			__raw_writeq(*(u64 *)from, to);
			from += 8;
			to += 8;
			count -= 8;
		}
	}

	while (count) {
		__raw_writeb(*(u8 *)from, to);
		from++;
		to++;
		count--;
	}
}

/*
 * "memset" on IO memory space.
 */
static inline
void __memset_io(volatile void __iomem *dst, int c, size_t count)
{
	u64 qc = (u8)c;

	qc |= qc << 8;
	qc |= qc << 16;
	qc |= qc << 32;

	while (count && !IS_ALIGNED((unsigned long)dst, 8)) {
		__raw_writeb(c, dst);
		dst++;
		count--;
	}

	while (count >= 8) {
		__raw_writeq(qc, dst);
		dst += 8;
		count -= 8;
	}

	while (count) {
		__raw_writeb(c, dst);
		dst++;
		count--;
	}
}
#endif /* CONFIG_ARM64 */

#ifdef CONFIG_ARM64
#define memset_io(a, b, c)		__memset_io((a), (b), (c))
#define memcpy_fromio(a, b, c)		__memcpy_fromio((a), (b), (c))
#define memcpy_toio(a, b, c)		__memcpy_toio((a), (b), (c))
#else
#define memset_io(a, b, c)		memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)		memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)		memcpy((void *)(a), (b), (c))
#endif

#include <asm-generic/io.h>
#include <iotrace.h>

#endif	/* __ASM_ARM_IO_H */
