// SPDX-License-Identifier: GPL-2.0-or-later
/* bit search implementation
 *
 * Copyright (C) 2004 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * Copyright (C) 2008 IBM Corporation
 * 'find_last_bit' is written by Rusty Russell <rusty@rustcorp.com.au>
 * (Inspired by David Howell's find_next_bit implementation)
 *
 * Rewritten by Yury Norov <yury.norov@gmail.com> to decrease
 * size and improve performance, 2015.
 */

#include <linux/bitops.h>
#include <linux/bitmap.h>
#include <linux/compat.h>
#include <linux/byteorder/swab.h>

/*
 * Common helper for find_bit() function family
 * @FETCH: The expression that fetches and pre-processes each word of bitmap(s)
 * @MUNGE: The expression that post-processes a word containing found bit (may be empty)
 * @size: The bitmap size in bits
 */
#define FIND_FIRST_BIT(FETCH, MUNGE, size)					\
({										\
	unsigned long idx, val, sz = (size);					\
										\
	for (idx = 0; idx * BITS_PER_LONG < sz; idx++) {			\
		val = (FETCH);							\
		if (val) {							\
			sz = min(idx * BITS_PER_LONG + __ffs(MUNGE(val)), sz);	\
			break;							\
		}								\
	}									\
										\
	sz;									\
})

/*
 * Common helper for find_next_bit() function family
 * @FETCH: The expression that fetches and pre-processes each word of bitmap(s)
 * @MUNGE: The expression that post-processes a word containing found bit (may be empty)
 * @size: The bitmap size in bits
 * @start: The bitnumber to start searching at
 */
#define FIND_NEXT_BIT(FETCH, MUNGE, size, start)				\
({										\
	unsigned long mask, idx, tmp, sz = (size), __start = (start);		\
										\
	if (unlikely(__start >= sz))						\
		goto out;							\
										\
	mask = MUNGE(BITMAP_FIRST_WORD_MASK(__start));				\
	idx = __start / BITS_PER_LONG;						\
										\
	for (tmp = (FETCH) & mask; !tmp; tmp = (FETCH)) {			\
		if ((idx + 1) * BITS_PER_LONG >= sz)				\
			goto out;						\
		idx++;								\
	}									\
										\
	sz = min(idx * BITS_PER_LONG + __ffs(MUNGE(tmp)), sz);			\
out:										\
	sz;									\
})


#ifndef find_first_bit
/*
 * Find the first set bit in a memory region.
 */
unsigned long _find_first_bit(const unsigned long *addr, unsigned long size)
{
	return FIND_FIRST_BIT(addr[idx], /* nop */, size);
}
EXPORT_SYMBOL(_find_first_bit);
#endif

#ifndef find_next_bit
unsigned long _find_next_bit(const unsigned long *addr, unsigned long nbits, unsigned long start)
{
	return FIND_NEXT_BIT(addr[idx], /* nop */, nbits, start);
}
EXPORT_SYMBOL(_find_next_bit);
#endif


#ifndef find_first_zero_bit
/*
 * Find the first cleared bit in a memory region.
 */
unsigned long _find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
	return FIND_FIRST_BIT(~addr[idx], /* nop */, size);
}
EXPORT_SYMBOL(_find_first_zero_bit);
#endif

#ifndef find_next_zero_bit
unsigned long _find_next_zero_bit(const unsigned long *addr, unsigned long nbits,
					 unsigned long start)
{
	return FIND_NEXT_BIT(~addr[idx], /* nop */, nbits, start);
}
EXPORT_SYMBOL(_find_next_zero_bit);
#endif

#ifdef __BIG_ENDIAN

#ifndef find_first_zero_bit_le
/*
 * Find the first cleared bit in an LE memory region.
 */
unsigned long _find_first_zero_bit_le(const unsigned long *addr, unsigned long size)
{
	return FIND_FIRST_BIT(~addr[idx], swab, size);
}
EXPORT_SYMBOL(_find_first_zero_bit_le);

#endif

#ifndef find_next_zero_bit_le
unsigned long _find_next_zero_bit_le(const unsigned long *addr,
					unsigned long size, unsigned long offset)
{
	return FIND_NEXT_BIT(~addr[idx], swab, size, offset);
}
EXPORT_SYMBOL(_find_next_zero_bit_le);
#endif

#ifndef find_next_bit_le
unsigned long _find_next_bit_le(const unsigned long *addr,
				unsigned long size, unsigned long offset)
{
	return FIND_NEXT_BIT(addr[idx], swab, size, offset);
}
EXPORT_SYMBOL(_find_next_bit_le);

#endif

#endif /* __BIG_ENDIAN */
