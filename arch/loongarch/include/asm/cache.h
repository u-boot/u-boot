/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef _ASM_LOONGARCH_CACHE_H
#define _ASM_LOONGARCH_CACHE_H

#include <linux/bitops.h>

/* cache */
void cache_flush(void);
void probe_caches(void);

#define cache_op(op, addr)						\
	__asm__ __volatile__(						\
	"	cacop	%0, %1					\n"	\
	:								\
	: "i" (op), "ZC" (*(unsigned char *)(addr)))

#define CACHE_MAX_LEVEL		3
#define CACHE_MAX_INDEX		6
#define CACHE_OP		GENMASK(4, 3)
#define  CACHE_INDEX_INVWB	0x1
#define  CACHE_HIT_INVWB	0x2
#define CACHE_INDEX		GENMASK(2, 0)

#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	32
#endif

#endif /* _ASM_LOONGARCH_CACHE_H */
