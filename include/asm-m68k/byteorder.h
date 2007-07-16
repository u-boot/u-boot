/*
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

#ifndef _M68K_BYTEORDER_H
#define _M68K_BYTEORDER_H

#include <asm/types.h>

#ifdef __GNUC__
#define __sw16(x) \
	((__u16)( \
		(((__u16)(x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(x) & (__u16)0xff00U) >> 8) ))
#define __sw32(x) \
	((__u32)( \
		(((__u32)(x)) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x)) >> 24) ))

extern __inline__ unsigned ld_le16(const volatile unsigned short *addr)
{
	unsigned result = *addr;
	return __sw16(result);
}

extern __inline__ void st_le16(volatile unsigned short *addr,
			       const unsigned val)
{
	*addr = __sw16(val);
}

extern __inline__ unsigned ld_le32(const volatile unsigned *addr)
{
	unsigned result = *addr;
	return __sw32(result);
}

extern __inline__ void st_le32(volatile unsigned *addr, const unsigned val)
{
	*addr = __sw32(val);
}

#if 0
/* alas, egcs sounds like it has a bug in this code that doesn't use the
   inline asm correctly, and can cause file corruption. Until I hear that
   it's fixed, I can live without the extra speed. I hope. */
#if !(__GNUC__ >= 2 && __GNUC_MINOR__ >= 90)
#if 0
#  define __arch_swab16(x) ld_le16(&x)
#  define __arch_swab32(x) ld_le32(&x)
#else
static __inline__ __attribute__ ((const))
__u16 ___arch__swab16(__u16 value)
{
	return __sw16(value);
}

static __inline__ __attribute__ ((const))
__u32 ___arch__swab32(__u32 value)
{
	return __sw32(value);
}

#define __arch__swab32(x) ___arch__swab32(x)
#define __arch__swab16(x) ___arch__swab16(x)
#endif				/* 0 */

#endif

/* The same, but returns converted value from the location pointer by addr. */
#define __arch__swab16p(addr) ld_le16(addr)
#define __arch__swab32p(addr) ld_le32(addr)

/* The same, but do the conversion in situ, ie. put the value back to addr. */
#define __arch__swab16s(addr) st_le16(addr,*addr)
#define __arch__swab32s(addr) st_le32(addr,*addr)
#endif

#endif				/* __GNUC__ */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define __BYTEORDER_HAS_U64__
#endif
#include <linux/byteorder/big_endian.h>

#endif				/* _M68K_BYTEORDER_H */
