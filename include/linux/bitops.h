#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H

#include <asm/types.h>

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

static inline int generic_ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int generic_fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}


/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

static inline unsigned int generic_hweight32(unsigned int w)
{
	unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
	res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
	return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}

static inline unsigned int generic_hweight16(unsigned int w)
{
	unsigned int res = (w & 0x5555) + ((w >> 1) & 0x5555);
	res = (res & 0x3333) + ((res >> 2) & 0x3333);
	res = (res & 0x0F0F) + ((res >> 4) & 0x0F0F);
	return (res & 0x00FF) + ((res >> 8) & 0x00FF);
}

static inline unsigned int generic_hweight8(unsigned int w)
{
	unsigned int res = (w & 0x55) + ((w >> 1) & 0x55);
	res = (res & 0x33) + ((res >> 2) & 0x33);
	return (res & 0x0F) + ((res >> 4) & 0x0F);
}

#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)

#include <asm/bitops.h>

/* linux/include/asm-generic/bitops/non-atomic.h */

#ifndef PLATFORM__SET_BIT
# define __set_bit generic_set_bit
#endif

#ifndef PLATFORM__CLEAR_BIT
# define __clear_bit generic_clear_bit
#endif

#ifndef PLATFORM_FFS
# define ffs generic_ffs
#endif

#ifndef PLATFORM_FLS
# define fls generic_fls
#endif

/**
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static inline void generic_set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p  |= mask;
}

static inline void generic_clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p &= ~mask;
}

#endif
