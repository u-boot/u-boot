/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * This is a Linux kernel compatibility layer for USB Gadget
 */

#ifndef __LIN_COMPAT_H__
#define __LIN_COMPAT_H__

#include <linux/bitops.h>
#include <linux/compat.h>

/* common */
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
