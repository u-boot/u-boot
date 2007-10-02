/*
 * IO header file
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#ifndef __ASM_M68K_IO_H__
#define __ASM_M68K_IO_H__

#include <asm/byteorder.h>

#define readb(addr)		in_8((volatile u8 *)(addr))
#define writeb(b,addr)		out_8((volatile u8 *)(addr), (b))
#if !defined(__BIG_ENDIAN)
#define readw(addr)		(*(volatile u16 *) (addr))
#define readl(addr)		(*(volatile u32 *) (addr))
#define writew(b,addr)		((*(volatile u16 *) (addr)) = (b))
#define writel(b,addr)		((*(volatile u32 *) (addr)) = (b))
#else
#define readw(addr)		in_le16((volatile u16 *)(addr))
#define readl(addr)		in_le32((volatile u32 *)(addr))
#define writew(b,addr)		out_le16((volatile u16 *)(addr),(b))
#define writel(b,addr)		out_le32((volatile u32 *)(addr),(b))
#endif

/*
 * The insw/outsw/insl/outsl macros don't do byte-swapping.
 * They are only used in practice for transferring buffers which
 * are arrays of bytes, and byte-swapping is not appropriate in
 * that case.  - paulus
 */
#define insb(port, buf, ns)	_insb((u8 *)((port)+_IO_BASE), (buf), (ns))
#define outsb(port, buf, ns)	_outsb((u8 *)((port)+_IO_BASE), (buf), (ns))
#define insw(port, buf, ns)	_insw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define outsw(port, buf, ns)	_outsw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define insl(port, buf, nl)	_insl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))
#define outsl(port, buf, nl)	_outsl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))

#define inb(port)		in_8((u8 *)((port)+_IO_BASE))
#define outb(val, port)		out_8((u8 *)((port)+_IO_BASE), (val))
#if !defined(__BIG_ENDIAN)
#define inw(port)		in_be16((u16 *)((port)+_IO_BASE))
#define outw(val, port)		out_be16((u16 *)((port)+_IO_BASE), (val))
#define inl(port)		in_be32((u32 *)((port)+_IO_BASE))
#define outl(val, port)		out_be32((u32 *)((port)+_IO_BASE), (val))
#else
#define inw(port)		in_le16((u16 *)((port)+_IO_BASE))
#define outw(val, port)		out_le16((u16 *)((port)+_IO_BASE), (val))
#define inl(port)		in_le32((u32 *)((port)+_IO_BASE))
#define outl(val, port)		out_le32((u32 *)((port)+_IO_BASE), (val))
#endif

extern inline void _insb(volatile u8 * port, void *buf, int ns)
{
	u8 *data = (u8 *) buf;
	while (ns--)
		*data++ = *port;
}

extern inline void _outsb(volatile u8 * port, const void *buf, int ns)
{
	u8 *data = (u8 *) buf;
	while (ns--)
		*port = *data++;
}

extern inline void _insw(volatile u16 * port, void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--)
		*data++ = __sw16(*port);
}

extern inline void _outsw(volatile u16 * port, const void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--) {
		*port = __sw16(*data);
		data++;
	}
}

extern inline void _insl(volatile u32 * port, void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--)
		*data++ = __sw32(*port);
}

extern inline void _outsl(volatile u32 * port, const void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--) {
		*port = __sw32(*data);
		data++;
	}
}

extern inline void _insw_ns(volatile u16 * port, void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--)
		*data++ = *port;
}

extern inline void _outsw_ns(volatile u16 * port, const void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--) {
		*port = *data++;
	}
}

extern inline void _insl_ns(volatile u32 * port, void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--)
		*data++ = *port;
}

extern inline void _outsl_ns(volatile u32 * port, const void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--) {
		*port = *data;
		data++;
	}
}

/*
 * The *_ns versions below don't do byte-swapping.
 * Neither do the standard versions now, these are just here
 * for older code.
 */
#define insw_ns(port, buf, ns)	_insw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define outsw_ns(port, buf, ns)	_outsw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define insl_ns(port, buf, nl)	_insl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))
#define outsl_ns(port, buf, nl)	_outsl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))

#define IO_SPACE_LIMIT ~0

/*
 * 8, 16 and 32 bit, big and little endian I/O operations, with barrier.
 */
extern inline int in_8(volatile u8 * addr)
{
	return (int)*addr;
}

extern inline void out_8(volatile u8 * addr, int val)
{
	*addr = (u8) val;
}

extern inline int in_le16(volatile u16 * addr)
{
	return __sw16(*addr);
}

extern inline int in_be16(volatile u16 * addr)
{
	return (*addr & 0xFFFF);
}

extern inline void out_le16(volatile u16 * addr, int val)
{
	*addr = __sw16(val);
}

extern inline void out_be16(volatile u16 * addr, int val)
{
	*addr = (u16) val;
}

extern inline unsigned in_le32(volatile u32 * addr)
{
	return __sw32(*addr);
}

extern inline unsigned in_be32(volatile u32 * addr)
{
	return (*addr);
}

extern inline void out_le32(volatile unsigned *addr, int val)
{
	*addr = __sw32(val);
}

extern inline void out_be32(volatile unsigned *addr, int val)
{
	*addr = val;
}

static inline void sync(void)
{
	/* This sync function is for PowerPC or other architecture instruction
	 * ColdFire does not have this instruction. Dummy function, added for
	 * compatibility (CFI driver)
	 */
}
#endif				/* __ASM_M68K_IO_H__ */
