/* originally from linux source.
 * removed the dependencies on CONFIG_ values
 * removed virt_to_phys stuff (and in fact everything surrounded by #if __KERNEL__)
 * Modified By Rob Taylor, Flying Pig Systems, 2000
 */

#ifndef _PPC_IO_H
#define _PPC_IO_H

#include <linux/config.h>
#include <asm/byteorder.h>

#define SIO_CONFIG_RA   0x398
#define SIO_CONFIG_RD   0x399


#define readb(addr) in_8((volatile u8 *)(addr))
#define writeb(b,addr) out_8((volatile u8 *)(addr), (b))
#if !defined(__BIG_ENDIAN)
#define readw(addr) (*(volatile u16 *) (addr))
#define readl(addr) (*(volatile u32 *) (addr))
#define writew(b,addr) ((*(volatile u16 *) (addr)) = (b))
#define writel(b,addr) ((*(volatile u32 *) (addr)) = (b))
#else
#define readw(addr) in_le16((volatile u16 *)(addr))
#define readl(addr) in_le32((volatile u32 *)(addr))
#define writew(b,addr) out_le16((volatile u16 *)(addr),(b))
#define writel(b,addr) out_le32((volatile u32 *)(addr),(b))
#endif

/*
 * The insw/outsw/insl/outsl macros don't do byte-swapping.
 * They are only used in practice for transferring buffers which
 * are arrays of bytes, and byte-swapping is not appropriate in
 * that case.  - paulus
 */
#define insb(port, buf, ns) _insb((u8 *)((port)+_IO_BASE), (buf), (ns))
#define outsb(port, buf, ns)    _outsb((u8 *)((port)+_IO_BASE), (buf), (ns))
#define insw(port, buf, ns) _insw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define outsw(port, buf, ns)    _outsw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define insl(port, buf, nl) _insl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))
#define outsl(port, buf, nl)    _outsl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))

#define inb(port)       in_8((u8 *)((port)+_IO_BASE))
#define outb(val, port)     out_8((u8 *)((port)+_IO_BASE), (val))
#if !defined(__BIG_ENDIAN)
#define inw(port)       in_be16((u16 *)((port)+_IO_BASE))
#define outw(val, port)     out_be16((u16 *)((port)+_IO_BASE), (val))
#define inl(port)       in_be32((u32 *)((port)+_IO_BASE))
#define outl(val, port)     out_be32((u32 *)((port)+_IO_BASE), (val))
#else
#define inw(port)       in_le16((u16 *)((port)+_IO_BASE))
#define outw(val, port)     out_le16((u16 *)((port)+_IO_BASE), (val))
#define inl(port)       in_le32((u32 *)((port)+_IO_BASE))
#define outl(val, port)     out_le32((u32 *)((port)+_IO_BASE), (val))
#endif

#define inb_p(port)     in_8((u8 *)((port)+_IO_BASE))
#define outb_p(val, port)   out_8((u8 *)((port)+_IO_BASE), (val))
#define inw_p(port)     in_le16((u16 *)((port)+_IO_BASE))
#define outw_p(val, port)   out_le16((u16 *)((port)+_IO_BASE), (val))
#define inl_p(port)     in_le32((u32 *)((port)+_IO_BASE))
#define outl_p(val, port)   out_le32((u32 *)((port)+_IO_BASE), (val))

extern void _insb(volatile u8 *port, void *buf, int ns);
extern void _outsb(volatile u8 *port, const void *buf, int ns);
extern void _insw(volatile u16 *port, void *buf, int ns);
extern void _outsw(volatile u16 *port, const void *buf, int ns);
extern void _insl(volatile u32 *port, void *buf, int nl);
extern void _outsl(volatile u32 *port, const void *buf, int nl);
extern void _insw_ns(volatile u16 *port, void *buf, int ns);
extern void _outsw_ns(volatile u16 *port, const void *buf, int ns);
extern void _insl_ns(volatile u32 *port, void *buf, int nl);
extern void _outsl_ns(volatile u32 *port, const void *buf, int nl);

/*
 * The *_ns versions below don't do byte-swapping.
 * Neither do the standard versions now, these are just here
 * for older code.
 */
#define insw_ns(port, buf, ns)  _insw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define outsw_ns(port, buf, ns) _outsw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define insl_ns(port, buf, nl)  _insl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))
#define outsl_ns(port, buf, nl) _outsl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))


#define IO_SPACE_LIMIT ~0

#define memset_io(a,b,c)       memset((void *)(a),(b),(c))
#define memcpy_fromio(a,b,c)   memcpy((a),(void *)(b),(c))
#define memcpy_toio(a,b,c)  memcpy((void *)(a),(b),(c))

/*
 * Enforce In-order Execution of I/O:
 * Acts as a barrier to ensure all previous I/O accesses have
 * completed before any further ones are issued.
 */
#define eieio() __asm__ __volatile__ ("eieio" : : : "memory");
#define sync()  __asm__ __volatile__ ("sync" : : : "memory");

/* Enforce in-order execution of data I/O.
 * No distinction between read/write on PPC; use eieio for all three.
 */
#define iobarrier_rw() eieio()
#define iobarrier_r()  eieio()
#define iobarrier_w()  eieio()

/*
 * 8, 16 and 32 bit, big and little endian I/O operations, with barrier.
 */
extern inline int in_8(volatile u8 *addr)
{
    int ret;

    __asm__ __volatile__("lbz%U1%X1 %0,%1; eieio" : "=r" (ret) : "m" (*addr));
    return ret;
}

extern inline void out_8(volatile u8 *addr, int val)
{
    __asm__ __volatile__("stb%U0%X0 %1,%0; eieio" : "=m" (*addr) : "r" (val));
}

extern inline int in_le16(volatile u16 *addr)
{
    int ret;

    __asm__ __volatile__("lhbrx %0,0,%1; eieio" : "=r" (ret) :
		  "r" (addr), "m" (*addr));
    return ret;
}

extern inline int in_be16(volatile u16 *addr)
{
    int ret;

    __asm__ __volatile__("lhz%U1%X1 %0,%1; eieio" : "=r" (ret) : "m" (*addr));
    return ret;
}

extern inline void out_le16(volatile u16 *addr, int val)
{
    __asm__ __volatile__("sthbrx %1,0,%2; eieio" : "=m" (*addr) :
		  "r" (val), "r" (addr));
}

extern inline void out_be16(volatile u16 *addr, int val)
{
    __asm__ __volatile__("sth%U0%X0 %1,%0; eieio" : "=m" (*addr) : "r" (val));
}

extern inline unsigned in_le32(volatile u32 *addr)
{
    unsigned ret;

    __asm__ __volatile__("lwbrx %0,0,%1; eieio" : "=r" (ret) :
		 "r" (addr), "m" (*addr));
    return ret;
}

extern inline unsigned in_be32(volatile u32 *addr)
{
    unsigned ret;

    __asm__ __volatile__("lwz%U1%X1 %0,%1; eieio" : "=r" (ret) : "m" (*addr));
    return ret;
}

extern inline void out_le32(volatile unsigned *addr, int val)
{
    __asm__ __volatile__("stwbrx %1,0,%2; eieio" : "=m" (*addr) :
		 "r" (val), "r" (addr));
}

extern inline void out_be32(volatile unsigned *addr, int val)
{
    __asm__ __volatile__("stw%U0%X0 %1,%0; eieio" : "=m" (*addr) : "r" (val));
}

#endif
