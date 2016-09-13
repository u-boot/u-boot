/*
 *  linux/include/asm-arm/atomic.h
 *
 *  Copyright (c) 1996 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Changelog:
 *   27-06-1996	RMK	Created
 *   13-04-1997	RMK	Made functions atomic!
 *   07-12-1997	RMK	Upgraded for v2.1.
 *   26-08-1998	PJB	Added #ifdef __KERNEL__
 */
#ifndef __ASM_ARM_ATOMIC_H
#define __ASM_ARM_ATOMIC_H

#ifdef CONFIG_SMP
#error SMP not supported
#endif

typedef struct { volatile int counter; } atomic_t;
#if BITS_PER_LONG == 32
typedef struct { volatile long long counter; } atomic64_t;
#else /* BIT_PER_LONG == 32 */
typedef struct { volatile long counter; } atomic64_t;
#endif

#define ATOMIC_INIT(i)	{ (i) }

#ifdef __KERNEL__
#include <asm/proc-armv/system.h>

#define atomic_read(v)	((v)->counter)
#define atomic_set(v, i)	(((v)->counter) = (i))
#define atomic64_read(v)	atomic_read(v)
#define atomic64_set(v, i)	atomic_set(v, i)

static inline void atomic_add(int i, volatile atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic_sub(int i, volatile atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}

static inline void atomic_inc(volatile atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += 1;
	local_irq_restore(flags);
}

static inline void atomic_dec(volatile atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= 1;
	local_irq_restore(flags);
}

static inline int atomic_dec_and_test(volatile atomic_t *v)
{
	unsigned long flags = 0;
	int val;

	local_irq_save(flags);
	val = v->counter;
	v->counter = val -= 1;
	local_irq_restore(flags);

	return val == 0;
}

static inline int atomic_add_negative(int i, volatile atomic_t *v)
{
	unsigned long flags = 0;
	int val;

	local_irq_save(flags);
	val = v->counter;
	v->counter = val += i;
	local_irq_restore(flags);

	return val < 0;
}

static inline void atomic_clear_mask(unsigned long mask, unsigned long *addr)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	*addr &= ~mask;
	local_irq_restore(flags);
}

#if BITS_PER_LONG == 32

static inline void atomic64_add(long long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic64_sub(long long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}

#else /* BIT_PER_LONG == 32 */

static inline void atomic64_add(long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic64_sub(long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}
#endif

static inline void atomic64_inc(volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += 1;
	local_irq_restore(flags);
}

static inline void atomic64_dec(volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= 1;
	local_irq_restore(flags);
}

/* Atomic operations are already serializing on ARM */
#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()

#endif
#endif
