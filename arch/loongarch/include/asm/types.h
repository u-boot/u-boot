/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Copyright (C) 2010 Shawn Lin (nobuhiro@andestech.com)
 * Copyright (C) 2011 Macpaul Lin (macpaul@andestech.com)
 * Copyright (C) 2017 Rick Chen (rick@andestech.com)
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef __ASM_LOONGARCH_TYPES_H
#define __ASM_LOONGARCH_TYPES_H

#include <asm-generic/int-ll64.h>

typedef unsigned short umode_t;

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

#define BITS_PER_LONG _LOONGARCH_SZLONG

#include <stddef.h>

#ifdef CONFIG_DMA_ADDR_T_64BIT
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif

typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;

#endif /* __KERNEL__ */

#endif
