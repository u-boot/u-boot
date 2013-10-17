/*
 * U-boot - bitops.h Routines for bit operations
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BLACKFIN_BITOPS_H
#define _BLACKFIN_BITOPS_H

/*
 * Copyright 1992, Linus Torvalds.
 */

#include <linux/config.h>
#include <asm/byteorder.h>
#include <asm/system.h>

#ifdef __KERNEL__
/*
 * Function prototypes to keep gcc -Wall happy
 */

/*
 * The __ functions are not atomic
 */

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
static __inline__ unsigned long ffz(unsigned long word)
{
	unsigned long result = 0;

	while (word & 1) {
		result++;
		word >>= 1;
	}
	return result;
}

static __inline__ void set_bit(int nr, volatile void *addr)
{
	int *a = (int *)addr;
	int mask;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a |= mask;
	local_irq_restore(flags);
}

static __inline__ void __set_bit(int nr, volatile void *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a |= mask;
}
#define PLATFORM__SET_BIT

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

static __inline__ void clear_bit(int nr, volatile void *addr)
{
	int *a = (int *)addr;
	int mask;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a &= ~mask;
	local_irq_restore(flags);
}

static __inline__ void change_bit(int nr, volatile void *addr)
{
	int mask, flags;
	unsigned long *ADDR = (unsigned long *)addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	local_irq_save(flags);
	*ADDR ^= mask;
	local_irq_restore(flags);
}

static __inline__ void __change_bit(int nr, volatile void *addr)
{
	int mask;
	unsigned long *ADDR = (unsigned long *)addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	*ADDR ^= mask;
}

static __inline__ int test_and_set_bit(int nr, volatile void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a |= mask;
	local_irq_restore(flags);

	return retval;
}

static __inline__ int __test_and_set_bit(int nr, volatile void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a |= mask;
	return retval;
}

static __inline__ int test_and_clear_bit(int nr, volatile void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	local_irq_restore(flags);

	return retval;
}

static __inline__ int __test_and_clear_bit(int nr, volatile void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	return retval;
}

static __inline__ int test_and_change_bit(int nr, volatile void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a ^= mask;
	local_irq_restore(flags);

	return retval;
}

static __inline__ int __test_and_change_bit(int nr, volatile void *addr)
{
	int mask, retval;
	volatile unsigned int *a = (volatile unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a ^= mask;
	return retval;
}

/*
 * This routine doesn't need to be atomic.
 */
static __inline__ int __constant_test_bit(int nr, const volatile void *addr)
{
	return ((1UL << (nr & 31)) &
		(((const volatile unsigned int *)addr)[nr >> 5])) != 0;
}

static __inline__ int __test_bit(int nr, volatile void *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *a) != 0);
}

#define	test_bit(nr,addr) \
(__builtin_constant_p(nr) ? \
 __constant_test_bit((nr),(addr)) : \
 __test_bit((nr),(addr)))

#define	find_first_zero_bit(addr, size) \
	find_next_zero_bit((addr), (size), 0)

static __inline__ int find_next_zero_bit(void *addr, int size, int offset)
{
	unsigned long *p = ((unsigned long *)addr) + (offset >> 5);
	unsigned long result = offset & ~31UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 31UL;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (32 - offset);
		if (size < 32)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= 32;
		result += 32;
	}
	while (size & ~31UL) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += 32;
		size -= 32;
	}
	if (!size)
		return result;
	tmp = *p;

      found_first:
	tmp |= ~0UL >> size;
      found_middle:
	return result + ffz(tmp);
}

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#define hweight32(x)	generic_hweight32(x)
#define hweight16(x)	generic_hweight16(x)
#define hweight8(x)	generic_hweight8(x)

static __inline__ int ext2_set_bit(int nr, volatile void *addr)
{
	int mask, retval;
	unsigned long flags;
	volatile unsigned char *ADDR = (unsigned char *)addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	local_irq_save(flags);
	retval = (mask & *ADDR) != 0;
	*ADDR |= mask;
	local_irq_restore(flags);
	return retval;
}

static __inline__ int ext2_clear_bit(int nr, volatile void *addr)
{
	int mask, retval;
	unsigned long flags;
	volatile unsigned char *ADDR = (unsigned char *)addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	local_irq_save(flags);
	retval = (mask & *ADDR) != 0;
	*ADDR &= ~mask;
	local_irq_restore(flags);
	return retval;
}

static __inline__ int ext2_test_bit(int nr, const volatile void *addr)
{
	int mask;
	const volatile unsigned char *ADDR = (const unsigned char *)addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	return ((mask & *ADDR) != 0);
}

#define ext2_find_first_zero_bit(addr, size) \
	ext2_find_next_zero_bit((addr), (size), 0)

static __inline__ unsigned long ext2_find_next_zero_bit(void *addr,
							unsigned long size,
							unsigned long offset)
{
	unsigned long *p = ((unsigned long *)addr) + (offset >> 5);
	unsigned long result = offset & ~31UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 31UL;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (32 - offset);
		if (size < 32)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= 32;
		result += 32;
	}
	while (size & ~31UL) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += 32;
		size -= 32;
	}
	if (!size)
		return result;
	tmp = *p;

      found_first:
	tmp |= ~0UL >> size;
      found_middle:
	return result + ffz(tmp);
}

/* Bitmap functions for the minix filesystem. */
#define minix_test_and_set_bit(nr,addr)		test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr)			set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr)	test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr)			test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size)	find_first_zero_bit(addr,size)

#endif

#endif
