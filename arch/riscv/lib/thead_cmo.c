// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <cpu_func.h>

/*
 * dcache.ipa rs1 (invalidate)
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000001    01010      rs1       000      00000  0001011
 *
 * dcache.cpa rs1 (clean)
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000001    01001      rs1       000      00000  0001011
 *
 * dcache.cipa rs1 (clean then invalidate)
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000001    01011      rs1       000      00000  0001011
 *
 * sync.s
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000000    11001     00000      000      00000  0001011
 */
#define DCACHE_IPA_A0	".long 0x02a5000b"
#define DCACHE_CPA_A0	".long 0x0295000b"
#define DCACHE_CIPA_A0	".long 0x02b5000b"

#define SYNC_S		".long 0x0190000b"

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		__asm__ __volatile__(DCACHE_IPA_A0);
	__asm__ __volatile__(SYNC_S);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		__asm__ __volatile__(DCACHE_CPA_A0);
	__asm__ __volatile__(SYNC_S);
}
