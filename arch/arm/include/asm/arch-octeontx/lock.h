/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 */

#ifndef __OCTEONTX2_LOCK_H__
#define __OCTEONTX2_LOCK_H__

/**
 * U-Boot doesn't implement spinlocks but we need them here since
 * we could be sharing with other processes (i.e. ATF)
 *
 * This is copied from the Linux kernel arm64 implementation.
 */
typedef struct octeontx_spinlock {
	u16 next;
	u16 owner;
} __aligned(4) octeontx_spinlock_t;

/**
 * Atomic compare and exchange, derived from atomic_ll_sc.h in Linux
 *
 * @param	ptr	pointer to 128 bits to compare and exchange
 * @param	old_hi	expected old value first 64 bits
 * @param	old_lo	expected old value second 64 bits
 * @param	new_hi	new first 64-bit value to set
 * @param	new_lo	new second 64-bit value to set
 * @param	last_hi	pointer to previous first 64-bit value
 * @param	last_lo	pointer previous second 64-bit value
 *
 * @return	true if successful, false if the old values mismatched
 *
 * last_hi and last_lo may be NULL.
 */
static inline bool octeontx_cmpxchg_atomic128(volatile void *ptr,
					      u64 old_hi, u64 old_lo,
					      u64 new_hi, u64 new_lo,
					      u64 *last_hi, u64 *last_lo)
{
	u64 tmp_hi, tmp_lo, oldval_hi, oldval_lo;
	u32 tmp;

	asm volatile (
		"	prfm	pstl1strm, %[v]				\n"
		"1:	ldaxp	%[oldval_lo], %[oldval_hi], %[v]	\n"
		"	eor	%[tmp_lo], %[oldval_lo], %[old_lo]	\n"
		"	eor	%[tmp_hi], %[oldval_hi], %[old_hi]	\n"
		"	cbnz	%[tmp_lo], 2f				\n"
		"	cbnz	%[tmp_hi], 2f				\n"
		"	stxp	%w[tmp],  %[new_hi], %[new_lo], %[v]	\n"
		"	cbnz	%w[tmp], 1b				\n"
		"2:							\n"
		: [tmp] "=&r" (tmp),
		  [oldval_lo] "=&r" (oldval_lo),
		  [oldval_hi] "=&r" (oldval_hi),
		  [v] "+Q" (*(u64 *)ptr),
		  [tmp_lo] "=&r" (tmp_lo), [tmp_hi] "=&r" (tmp_hi)
		: [old_lo] "Lr" (old_lo), [old_hi] "Lr" (old_hi),
		  [new_lo] "r" (new_lo), [new_hi] "r" (new_hi)
		: "memory");
	if (last_hi)
		*last_hi = oldval_hi;
	if (last_lo)
		*last_lo = oldval_lo;
	return !(tmp_hi | tmp_lo);
}

static inline void octeontx_init_spin_lock(octeontx_spinlock_t *lock)
{
	*(u32 *)lock = 0;
	__iowmb();
}

/**
 * Acquires a spinlock
 *
 * @param	lock	pointer to lock
 *
 * This code is copied from the Linux aarch64 spinlock.h file
 * and is compatible with it.
 */
static inline void octeontx_spin_lock(octeontx_spinlock_t *lock)
{
	unsigned int tmp;
	octeontx_spinlock_t lockval, newval;

	asm volatile (
	/* Atomically increment the next ticket. */
	/* LL/SC */
"	prfm	pstl1strm, %3		\n"
"1:	ldaxr	%w0, %3			\n"
"	add	%w1, %w0, %w5		\n"
"	stxr	%w2, %w1, %3		\n"
"	cbnz	%w2, 1b			\n"

	/* Did we get the lock? */
"	eor	%w1, %w0, %w0, ror #16	\n"
"	cbz	%w1, 3f			\n"
	/*
	 * No: spin on the owner. Send a local event to avoid missing an
	 * unlock before the exclusive load.
	 */
"	sevl				\n"
"2:	wfe				\n"
"	ldaxrh	%w2, %4			\n"
"	eor	%w1, %w2, %w0, lsr #16	\n"
"	cbnz	%w1, 2b			\n"
	/* We got the lock. Critical section starts here. */
"3:"
	: "=&r" (lockval), "=&r" (newval), "=&r" (tmp), "+Q" (*lock)
	: "Q" (lock->owner), "I" (1 << 16)
	: "memory");
}

/**
 * Releases a spinlock
 *
 * @param	lock	pointer to lock
 *
 * This code is copied from the Linux aarch64 spinlock.h file.
 */
static inline void octeontx_spin_unlock(octeontx_spinlock_t *lock)
{
	unsigned long tmp;

	asm volatile (
		"	ldrh	%w1, %0		\n"
		"	add	%w1, %w1, #1	\n"
		"	stlrh	%w1, %0		\n"
		"	nop			\n"
		: "=Q" (lock->owner), "=&r" (tmp)
		:
		: "memory"

	);
}

#endif /* __OCTEONTX2_LOCK_H__ */
