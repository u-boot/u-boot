/*
 * include/asm-microblaze/byteorder.h -- Endian id and conversion ops
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_BYTEORDER_H__
#define __MICROBLAZE_BYTEORDER_H__

#include <asm/types.h>

#ifdef __GNUC__

/* This is effectively a dupe of the arch-independent byteswap
   code in include/linux/byteorder/swab.h, however we force a cast
   of the result up to 32 bits.  This in turn forces the compiler
   to explicitly clear the high 16 bits, which it wasn't doing otherwise.

   I think this is a symptom of a bug in mb-gcc.  JW 20040303
*/


static __inline__ __u16 ___arch__swab16 (__u16 half_word)
{
	/* 32 bit temp to cast result, forcing clearing of high word */
	__u32 temp;

	temp = ((half_word & 0x00FFU) << 8) | ((half_word & 0xFF00U) >> 8);

	return (__u16) temp;
}

#define __arch__swab16(x) ___arch__swab16(x)

/* Microblaze has no arch-specific endian conversion insns */

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#endif /* __GNUC__ */

#ifdef __MICROBLAZEEL__
#include <linux/byteorder/little_endian.h>
#else
#include <linux/byteorder/big_endian.h>
#endif

#endif /* __MICROBLAZE_BYTEORDER_H__ */
