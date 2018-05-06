/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_ATOMIC_H
#define _XTENSA_ATOMIC_H

#include <asm/system.h>

typedef struct { volatile int counter; } atomic_t;

#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	((v)->counter = (i))

static inline void atomic_add(int i, atomic_t *v)
{
	unsigned long flags;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic_sub(int i, atomic_t *v)
{
	unsigned long flags;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}

static inline void atomic_inc(atomic_t *v)
{
	unsigned long flags;

	local_irq_save(flags);
	++v->counter;
	local_irq_restore(flags);
}

static inline void atomic_dec(atomic_t *v)
{
	unsigned long flags;

	local_irq_save(flags);
	--v->counter;
	local_irq_restore(flags);
}

#endif
