/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/cacheops.h>
#include <asm/mipsregs.h>

static inline unsigned long icache_line_size(void)
{
	unsigned long conf1, il;

	if (!config_enabled(CONFIG_SYS_CACHE_SIZE_AUTO))
		return CONFIG_SYS_ICACHE_LINE_SIZE;

	conf1 = read_c0_config1();
	il = (conf1 & MIPS_CONF1_IL) >> MIPS_CONF1_IL_SHF;
	if (!il)
		return 0;
	return 2 << il;
}

static inline unsigned long dcache_line_size(void)
{
	unsigned long conf1, dl;

	if (!config_enabled(CONFIG_SYS_CACHE_SIZE_AUTO))
		return CONFIG_SYS_DCACHE_LINE_SIZE;

	conf1 = read_c0_config1();
	dl = (conf1 & MIPS_CONF1_DL) >> MIPS_CONF1_DL_SHF;
	if (!dl)
		return 0;
	return 2 << dl;
}

#define cache_loop(start, end, lsize, ops...) do {			\
	const void *addr = (const void *)(start & ~(lsize - 1));	\
	const void *aend = (const void *)((end - 1) & ~(lsize - 1));	\
	const unsigned int cache_ops[] = { ops };			\
	unsigned int i;							\
									\
	for (; addr <= aend; addr += lsize) {				\
		for (i = 0; i < ARRAY_SIZE(cache_ops); i++)		\
			mips_cache(cache_ops[i], addr);			\
	}								\
} while (0)

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long ilsize = icache_line_size();
	unsigned long dlsize = dcache_line_size();

	/* aend will be miscalculated when size is zero, so we return here */
	if (size == 0)
		return;

	if (ilsize == dlsize) {
		/* flush I-cache & D-cache simultaneously */
		cache_loop(start_addr, start_addr + size, ilsize,
			   HIT_WRITEBACK_INV_D, HIT_INVALIDATE_I);
		return;
	}

	/* flush D-cache */
	cache_loop(start_addr, start_addr + size, dlsize, HIT_WRITEBACK_INV_D);

	/* flush I-cache */
	cache_loop(start_addr, start_addr + size, ilsize, HIT_INVALIDATE_I);
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();

	/* aend will be miscalculated when size is zero, so we return here */
	if (start_addr == stop)
		return;

	cache_loop(start_addr, stop, lsize, HIT_WRITEBACK_INV_D);
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();

	/* aend will be miscalculated when size is zero, so we return here */
	if (start_addr == stop)
		return;

	cache_loop(start_addr, stop, lsize, HIT_INVALIDATE_D);
}
