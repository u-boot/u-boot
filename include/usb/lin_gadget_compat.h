/*
 * Copyright (c) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * This is a Linux kernel compatibility layer for USB Gadget
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LIN_COMPAT_H__
#define __LIN_COMPAT_H__

#include <linux/compat.h>

/* common */
#define spin_lock_init(...)
#define spin_lock(...)
#define spin_lock_irqsave(lock, flags) do { debug("%lu\n", flags); } while (0)
#define spin_unlock(...)
#define spin_unlock_irqrestore(lock, flags) do {flags = 0; } while (0)
#define disable_irq(...)
#define enable_irq(...)

#define mutex_init(...)
#define mutex_lock(...)
#define mutex_unlock(...)

#define GFP_KERNEL	0

#define IRQ_HANDLED	1

#define ENOTSUPP	524	/* Operation is not supported */

#define BITS_PER_BYTE				8
#define BITS_TO_LONGS(nr) \
	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define DECLARE_BITMAP(name, bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

#define small_const_nbits(nbits) \
	(__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)

static inline void bitmap_zero(unsigned long *dst, int nbits)
{
	if (small_const_nbits(nbits))
		*dst = 0UL;
	else {
		int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memset(dst, 0, len);
	}
}

#define dma_cache_maint(addr, size, mode) cache_flush()
void cache_flush(void);

#endif /* __LIN_COMPAT_H__ */
