/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __ASM_SANDBOX_TYPES_H
#define __ASM_SANDBOX_TYPES_H

#include <asm-generic/int-ll64.h>

typedef unsigned short umode_t;

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

/*
 * Number of bits in a C 'long' on this architecture.
 */
#ifdef	CONFIG_PHYS64
#define BITS_PER_LONG 64
#else	/* CONFIG_PHYS64 */
#define BITS_PER_LONG 32
#endif	/* CONFIG_PHYS64 */

#ifdef	CONFIG_PHYS64
typedef unsigned long long dma_addr_t;
typedef u64 phys_addr_t;
typedef u64 phys_size_t;
#else	/* CONFIG_PHYS64 */
typedef unsigned long dma_addr_t;
typedef u32 phys_addr_t;
typedef u32 phys_size_t;
#endif	/* CONFIG_PHYS64 */

#endif /* __KERNEL__ */

#endif
