/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_FIND_H_
#define __LINUX_FIND_H_

#include <linux/bitops.h>

unsigned long _find_next_bit(const unsigned long *addr1, unsigned long nbits,
				unsigned long start);
unsigned long _find_next_zero_bit(const unsigned long *addr, unsigned long nbits,
					 unsigned long start);
unsigned long _find_first_bit(const unsigned long *addr, unsigned long size);
unsigned long _find_first_zero_bit(const unsigned long *addr, unsigned long size);

#ifdef __BIG_ENDIAN
unsigned long _find_first_zero_bit_le(const unsigned long *addr, unsigned long size);
unsigned long _find_next_zero_bit_le(const  unsigned long *addr, unsigned
					long size, unsigned long offset);
unsigned long _find_next_bit_le(const unsigned long *addr, unsigned
				long size, unsigned long offset);
#endif

#ifndef find_next_bit
/**
 * find_next_bit - find the next set bit in a memory region
 * @addr: The address to base the search on
 * @size: The bitmap size in bits
 * @offset: The bitnumber to start searching at
 *
 * Returns the bit number for the next set bit
 * If no bits are set, returns @size.
 */
static inline
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
			    unsigned long offset)
{
	if (small_const_nbits(size)) {
		unsigned long val;

		if (unlikely(offset >= size))
			return size;

		val = *addr & GENMASK(size - 1, offset);
		return val ? __ffs(val) : size;
	}

	return _find_next_bit(addr, size, offset);
}
#endif

#ifndef find_next_zero_bit
/**
 * find_next_zero_bit - find the next cleared bit in a memory region
 * @addr: The address to base the search on
 * @size: The bitmap size in bits
 * @offset: The bitnumber to start searching at
 *
 * Returns the bit number of the next zero bit
 * If no bits are zero, returns @size.
 */
static inline
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
				 unsigned long offset)
{
	if (small_const_nbits(size)) {
		unsigned long val;

		if (unlikely(offset >= size))
			return size;

		val = *addr | ~GENMASK(size - 1, offset);
		return val == ~0UL ? size : ffz(val);
	}

	return _find_next_zero_bit(addr, size, offset);
}
#endif


#ifndef find_first_bit
/**
 * find_first_bit - find the first set bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum number of bits to search
 *
 * Returns the bit number of the first set bit.
 * If no bits are set, returns @size.
 */
static inline
unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
	if (small_const_nbits(size)) {
		unsigned long val = *addr & GENMASK(size - 1, 0);

		return val ? __ffs(val) : size;
	}

	return _find_first_bit(addr, size);
}
#endif

#ifndef find_first_zero_bit
/**
 * find_first_zero_bit - find the first cleared bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum number of bits to search
 *
 * Returns the bit number of the first cleared bit.
 * If no bits are zero, returns @size.
 */
static inline
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
	if (small_const_nbits(size)) {
		unsigned long val = *addr | ~GENMASK(size - 1, 0);

		return val == ~0UL ? size : ffz(val);
	}

	return _find_first_zero_bit(addr, size);
}
#endif

#if defined(__LITTLE_ENDIAN)

static inline unsigned long find_next_zero_bit_le(const void *addr,
		unsigned long size, unsigned long offset)
{
	return find_next_zero_bit(addr, size, offset);
}

static inline unsigned long find_next_bit_le(const void *addr,
		unsigned long size, unsigned long offset)
{
	return find_next_bit(addr, size, offset);
}

static inline unsigned long find_first_zero_bit_le(const void *addr,
		unsigned long size)
{
	return find_first_zero_bit(addr, size);
}

#elif defined(__BIG_ENDIAN)

#ifndef find_next_zero_bit_le
static inline
unsigned long find_next_zero_bit_le(const void *addr, unsigned
		long size, unsigned long offset)
{
	if (small_const_nbits(size)) {
		unsigned long val = *(const unsigned long *)addr;

		if (unlikely(offset >= size))
			return size;

		val = swab(val) | ~GENMASK(size - 1, offset);
		return val == ~0UL ? size : ffz(val);
	}

	return _find_next_zero_bit_le(addr, size, offset);
}
#endif

#ifndef find_first_zero_bit_le
static inline
unsigned long find_first_zero_bit_le(const void *addr, unsigned long size)
{
	if (small_const_nbits(size)) {
		unsigned long val = swab(*(const unsigned long *)addr) | ~GENMASK(size - 1, 0);

		return val == ~0UL ? size : ffz(val);
	}

	return _find_first_zero_bit_le(addr, size);
}
#endif

#ifndef find_next_bit_le
static inline
unsigned long find_next_bit_le(const void *addr, unsigned
		long size, unsigned long offset)
{
	if (small_const_nbits(size)) {
		unsigned long val = *(const unsigned long *)addr;

		if (unlikely(offset >= size))
			return size;

		val = swab(val) & GENMASK(size - 1, offset);
		return val ? __ffs(val) : size;
	}

	return _find_next_bit_le(addr, size, offset);
}
#endif

#endif

#endif /*__LINUX_FIND_H_ */