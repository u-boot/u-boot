/*
 * Copyright 1995, Russell King.
 * Various bits and pieces copyrights include:
 * Linus Torvalds (test_bit).
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 *
 * Please note that the code in this file should never be included
 * from user space.  Many of these are not implemented in assembler
 * since they would be too costly.  Also, they require priviledged
 * instructions (which are not available from user mode) to ensure
 * that they are atomic.
 */

#ifndef __ASM_RISCV_BITOPS_H
#define __ASM_RISCV_BITOPS_H

#ifdef __KERNEL__

#include <asm/system.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/__ffs.h>

#define smp_mb__before_clear_bit()	do { } while (0)
#define smp_mb__after_clear_bit()	do { } while (0)

/*
 * Function prototypes to keep gcc -Wall happy.
 */
static inline void __set_bit(int nr, void *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a |= mask;
}

#define PLATFORM__SET_BIT

static inline void __clear_bit(int nr, void *addr)
{
	int *a = (int *)addr;
	int mask;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a &= ~mask;
}

#define PLATFORM__CLEAR_BIT

static inline void __change_bit(int nr, void *addr)
{
	int mask;
	unsigned long *ADDR = (unsigned long *)addr;

	ADDR += nr >> 5;
	mask = 1 << (nr & 31);
	*ADDR ^= mask;
}

static inline int __test_and_set_bit(int nr, void *addr)
{
	int mask, retval;
	unsigned int *a = (unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a |= mask;
	return retval;
}

static inline int __test_and_clear_bit(int nr, void *addr)
{
	int mask, retval;
	unsigned int *a = (unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	return retval;
}

static inline int __test_and_change_bit(int nr, void *addr)
{
	int mask, retval;
	unsigned int *a = (unsigned int *)addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a ^= mask;
	return retval;
}

/*
 * This routine doesn't need to be atomic.
 */
static inline int test_bit(int nr, const void *addr)
{
	return ((unsigned char *)addr)[nr >> 3] & (1U << (nr & 7));
}

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
static inline unsigned long ffz(unsigned long word)
{
	int k;

	word = ~word;
	k = 31;
	if (word & 0x0000ffff) {
		k -= 16; word <<= 16;
	}
	if (word & 0x00ff0000) {
		k -= 8;  word <<= 8;
	}
	if (word & 0x0f000000) {
		k -= 4;  word <<= 4;
	}
	if (word & 0x30000000) {
		k -= 2;  word <<= 2;
	}
	if (word & 0x40000000)
		k -= 1;

	return k;
}

static inline int find_next_zero_bit(void *addr, int size, int offset)
{
	unsigned long *p = ((unsigned long *)addr) + (offset / BITS_PER_LONG);
	unsigned long result = offset & ~(BITS_PER_LONG - 1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= (BITS_PER_LONG - 1);
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (BITS_PER_LONG - offset);
		if (size < BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG - 1)) {
		tmp = *(p++);
		if (~tmp)
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
found_middle:
	return result + ffz(tmp);
}

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

/*
 * redefined in include/linux/bitops.h
 * #define ffs(x) generic_ffs(x)
 */

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#define find_first_zero_bit(addr, size) \
	find_next_zero_bit((addr), (size), 0)

#define test_and_set_bit		__test_and_set_bit
#define test_and_clear_bit		__test_and_clear_bit

#define ext2_set_bit			test_and_set_bit
#define ext2_clear_bit			test_and_clear_bit
#define ext2_test_bit			test_bit
#define ext2_find_first_zero_bit	find_first_zero_bit
#define ext2_find_next_zero_bit		find_next_zero_bit

/* Bitmap functions for the minix filesystem. */
#define minix_test_and_set_bit(nr, addr)	test_and_set_bit(nr, addr)
#define minix_set_bit(nr, addr)			set_bit(nr, addr)
#define minix_test_and_clear_bit(nr, addr)	test_and_clear_bit(nr, addr)
#define minix_test_bit(nr, addr)		test_bit(nr, addr)
#define minix_find_first_zero_bit(addr, size)	find_first_zero_bit(addr, size)

#endif /* __KERNEL__ */

#endif /* __ASM_RISCV_BITOPS_H */
