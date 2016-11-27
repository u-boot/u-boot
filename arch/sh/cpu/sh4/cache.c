/*
 * (C) Copyright 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/system.h>

#define CACHE_VALID       1
#define CACHE_UPDATED     2

static inline void cache_wback_all(void)
{
	unsigned long addr, data, i, j;

	for (i = 0; i < CACHE_OC_NUM_ENTRIES; i++) {
		for (j = 0; j < CACHE_OC_NUM_WAYS; j++) {
			addr = CACHE_OC_ADDRESS_ARRAY
				| (j << CACHE_OC_WAY_SHIFT)
				| (i << CACHE_OC_ENTRY_SHIFT);
			data = inl(addr);
			if (data & CACHE_UPDATED) {
				data &= ~CACHE_UPDATED;
				outl(data, addr);
			}
		}
	}
}

#define CACHE_ENABLE      0
#define CACHE_DISABLE     1

int cache_control(unsigned int cmd)
{
	unsigned long ccr;

	jump_to_P2();
	ccr = inl(CCR);

	if (ccr & CCR_CACHE_ENABLE)
		cache_wback_all();

	if (cmd == CACHE_DISABLE)
		outl(CCR_CACHE_STOP, CCR);
	else
		outl(CCR_CACHE_INIT, CCR);
	back_to_P1();

	return 0;
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	u32 v;

	start &= ~(L1_CACHE_BYTES - 1);
	for (v = start; v < end; v += L1_CACHE_BYTES) {
		asm volatile ("ocbp     %0" :	/* no output */
			      : "m" (__m(v)));
	}
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	u32 v;

	start &= ~(L1_CACHE_BYTES - 1);
	for (v = start; v < end; v += L1_CACHE_BYTES) {
		asm volatile ("ocbi     %0" :	/* no output */
			      : "m" (__m(v)));
	}
}
