/*
 *  linux/include/asm-arm/io.h
 *
 *  Copyright (C) 1996-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Modifications:
 *  16-Sep-1996	RMK	Inlined the inx/outx functions & optimised for both
 *			constant addresses and variable addresses.
 *  04-Dec-1997	RMK	Moved a lot of this stuff to the new architecture
 *			specific IO header files.
 *  27-Mar-1999	PJB	Second parameter of memcpy_toio is const..
 *  04-Apr-1999	PJB	Added check_signature.
 *  12-Dec-1999	RMK	More cleanups
 *  18-Jun-2000 RMK	Removed virt_to_* and friends definitions
 */
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <linux/types.h>
#include <asm/byteorder.h>
#include <asm/memory.h>
#include <asm/arch/hardware.h>

/*
 * Generic virtual read/write.  Note that we don't support half-word
 * read/writes.  We define __arch_*[bl] here, and leave __arch_*w
 * to the architecture specific code.
 */
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getl(a)			(*(volatile unsigned int  *)(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int  *)(a) = (v))

extern void __raw_writesb(unsigned int addr, const void *data, int bytelen);
extern void __raw_writesw(unsigned int addr, const void *data, int wordlen);
extern void __raw_writesl(unsigned int addr, const void *data, int longlen);

extern void __raw_readsb(unsigned int addr, void *data, int bytelen);
extern void __raw_readsw(unsigned int addr, void *data, int wordlen);
extern void __raw_readsl(unsigned int addr, void *data, int longlen);

#define __raw_writeb(v,a)		__arch_putb(v,a)
#define __raw_writew(v,a)		__arch_putw(v,a)
#define __raw_writel(v,a)		__arch_putl(v,a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)

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
 * Now, pick up the machine-defined IO definitions
 */
#include <asm/arch/io.h>

/*
 * IO definitions.  We define {out,in,outs,ins}[bwl] if __io is defined
 * by the machine.  Otherwise, these definitions are left for the machine
 * specific header files to pick up.
 *
 * Note that we prevent GCC re-ordering or caching values in expressions
 * by introducing sequence points into the in*() definitions.  Note that
 * __raw_* do not guarantee this behaviour.
 */
#ifdef __io
#define outb(v,p)			__raw_writeb(v,__io(p))
#define outw(v,p)			__raw_writew(v,__io(p))
#define outl(v,p)			__raw_writel(v,__io(p))

#define inb(p)		({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)		({ unsigned int __v = __raw_readw(__io(p)); __v; })
#define inl(p)		({ unsigned int __v = __raw_readl(__io(p)); __v; })

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

/*
 * ioremap and friends.
 *
 * ioremap takes a PCI memory address, as specified in
 * linux/Documentation/IO-mapping.txt.  If you want a
 * physical address, use __ioremap instead.
 */
extern void * __ioremap(unsigned long offset, size_t size, unsigned long flags);
extern void __iounmap(void *addr);

/*
 * Generic ioremap support.
 *
 * Define:
 *  iomem_valid_addr(off,size)
 *  iomem_to_phys(off)
 */
#ifdef iomem_valid_addr
#define __arch_ioremap(off,sz,nocache)				\
 ({								\
	unsigned long _off = (off), _size = (sz);		\
	void *_ret = (void *)0;					\
	if (iomem_valid_addr(_off, _size))			\
		_ret = __ioremap(iomem_to_phys(_off),_size,0);	\
	_ret;							\
 })

#define __arch_iounmap __iounmap
#endif

#define ioremap(off,sz)			__arch_ioremap((off),(sz),0)
#define ioremap_nocache(off,sz)		__arch_ioremap((off),(sz),1)
#define iounmap(_addr)			__arch_iounmap(_addr)

/*
 * DMA-consistent mapping functions.  These allocate/free a region of
 * uncached, unwrite-buffered mapped memory space for use with DMA
 * devices.  This is the "generic" version.  The PCI specific version
 * is in pci.h
 */
extern void *consistent_alloc(int gfp, size_t size, dma_addr_t *handle);
extern void consistent_free(void *vaddr, size_t size, dma_addr_t handle);
extern void consistent_sync(void *vaddr, size_t size, int rw);

/*
 * String version of IO memory access ops:
 */
extern void _memcpy_fromio(void *, unsigned long, size_t);
extern void _memcpy_toio(unsigned long, const void *, size_t);
extern void _memset_io(unsigned long, int, size_t);

extern void __readwrite_bug(const char *fn);

/*
 * If this architecture has PCI memory IO, then define the read/write
 * macros.  These should only be used with the cookie passed from
 * ioremap.
 */
#ifdef __mem_pci

#define readb(addr) ({ unsigned int __v = __raw_readb(__mem_pci(addr)); __v; })
#define readw(addr) ({ unsigned int __v = __raw_readw(__mem_pci(addr)); __v; })
#define readl(addr) ({ unsigned int __v = __raw_readl(__mem_pci(addr)); __v; })

#define writeb(val,addr)		__raw_writeb(val,__mem_pci(addr))
#define writew(val,addr)		__raw_writew(val,__mem_pci(addr))
#define writel(val,addr)		__raw_writel(val,__mem_pci(addr))

#define memset_io(a,b,c)		_memset_io(__mem_pci(a),(b),(c))
#define memcpy_fromio(a,b,c)		_memcpy_fromio((a),__mem_pci(b),(c))
#define memcpy_toio(a,b,c)		_memcpy_toio(__mem_pci(a),(b),(c))

#define eth_io_copy_and_sum(a,b,c,d) \
				eth_copy_and_sum((a),__mem_pci(b),(c),(d))

static inline int
check_signature(unsigned long io_addr, const unsigned char *signature,
		int length)
{
	int retval = 0;
	do {
		if (readb(io_addr) != *signature)
			goto out;
		io_addr++;
		signature++;
		length--;
	} while (length);
	retval = 1;
out:
	return retval;
}

#elif !defined(readb)

#define readb(addr)			(__readwrite_bug("readb"),0)
#define readw(addr)			(__readwrite_bug("readw"),0)
#define readl(addr)			(__readwrite_bug("readl"),0)
#define writeb(v,addr)			__readwrite_bug("writeb")
#define writew(v,addr)			__readwrite_bug("writew")
#define writel(v,addr)			__readwrite_bug("writel")

#define eth_io_copy_and_sum(a,b,c,d)	__readwrite_bug("eth_io_copy_and_sum")

#define check_signature(io,sig,len)	(0)

#endif	/* __mem_pci */

/*
 * remap a physical address `phys' of size `size' with page protection `prot'
 * into virtual address `from'
 */
#define io_remap_page_range(from,phys,size,prot) \
		remap_page_range(from,phys,size,prot)


/*
 * If this architecture has ISA IO, then define the isa_read/isa_write
 * macros.
 */
#ifdef __mem_isa

#define isa_readb(addr)			__raw_readb(__mem_isa(addr))
#define isa_readw(addr)			__raw_readw(__mem_isa(addr))
#define isa_readl(addr)			__raw_readl(__mem_isa(addr))
#define isa_writeb(val,addr)		__raw_writeb(val,__mem_isa(addr))
#define isa_writew(val,addr)		__raw_writew(val,__mem_isa(addr))
#define isa_writel(val,addr)		__raw_writel(val,__mem_isa(addr))
#define isa_memset_io(a,b,c)		_memset_io(__mem_isa(a),(b),(c))
#define isa_memcpy_fromio(a,b,c)	_memcpy_fromio((a),__mem_isa(b),(c))
#define isa_memcpy_toio(a,b,c)		_memcpy_toio(__mem_isa((a)),(b),(c))

#define isa_eth_io_copy_and_sum(a,b,c,d) \
				eth_copy_and_sum((a),__mem_isa(b),(c),(d))

static inline int
isa_check_signature(unsigned long io_addr, const unsigned char *signature,
		    int length)
{
	int retval = 0;
	do {
		if (isa_readb(io_addr) != *signature)
			goto out;
		io_addr++;
		signature++;
		length--;
	} while (length);
	retval = 1;
out:
	return retval;
}

#else	/* __mem_isa */

#define isa_readb(addr)			(__readwrite_bug("isa_readb"),0)
#define isa_readw(addr)			(__readwrite_bug("isa_readw"),0)
#define isa_readl(addr)			(__readwrite_bug("isa_readl"),0)
#define isa_writeb(val,addr)		__readwrite_bug("isa_writeb")
#define isa_writew(val,addr)		__readwrite_bug("isa_writew")
#define isa_writel(val,addr)		__readwrite_bug("isa_writel")
#define isa_memset_io(a,b,c)		__readwrite_bug("isa_memset_io")
#define isa_memcpy_fromio(a,b,c)	__readwrite_bug("isa_memcpy_fromio")
#define isa_memcpy_toio(a,b,c)		__readwrite_bug("isa_memcpy_toio")

#define isa_eth_io_copy_and_sum(a,b,c,d) \
				__readwrite_bug("isa_eth_io_copy_and_sum")

#define isa_check_signature(io,sig,len)	(0)

#endif	/* __mem_isa */

#endif	/* __ASM_ARM_IO_H */
