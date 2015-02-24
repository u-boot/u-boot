/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/cacheops.h>
#include <asm/mipsregs.h>

#ifdef CONFIG_SYS_CACHELINE_SIZE

static inline unsigned long icache_line_size(void)
{
	return CONFIG_SYS_CACHELINE_SIZE;
}

static inline unsigned long dcache_line_size(void)
{
	return CONFIG_SYS_CACHELINE_SIZE;
}

#else /* !CONFIG_SYS_CACHELINE_SIZE */

static inline unsigned long icache_line_size(void)
{
	unsigned long conf1, il;
	conf1 = read_c0_config1();
	il = (conf1 & MIPS_CONF1_IL) >> MIPS_CONF1_IL_SHIFT;
	if (!il)
		return 0;
	return 2 << il;
}

static inline unsigned long dcache_line_size(void)
{
	unsigned long conf1, dl;
	conf1 = read_c0_config1();
	dl = (conf1 & MIPS_CONF1_DL) >> MIPS_CONF1_DL_SHIFT;
	if (!dl)
		return 0;
	return 2 << dl;
}

#endif /* !CONFIG_SYS_CACHELINE_SIZE */

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long ilsize = icache_line_size();
	unsigned long dlsize = dcache_line_size();
	const void *addr, *aend;

	/* aend will be miscalculated when size is zero, so we return here */
	if (size == 0)
		return;

	addr = (const void *)(start_addr & ~(dlsize - 1));
	aend = (const void *)((start_addr + size - 1) & ~(dlsize - 1));

	if (ilsize == dlsize) {
		/* flush I-cache & D-cache simultaneously */
		while (1) {
			mips_cache(HIT_WRITEBACK_INV_D, addr);
			mips_cache(HIT_INVALIDATE_I, addr);
			if (addr == aend)
				break;
			addr += dlsize;
		}
		return;
	}

	/* flush D-cache */
	while (1) {
		mips_cache(HIT_WRITEBACK_INV_D, addr);
		if (addr == aend)
			break;
		addr += dlsize;
	}

	/* flush I-cache */
	addr = (const void *)(start_addr & ~(ilsize - 1));
	aend = (const void *)((start_addr + size - 1) & ~(ilsize - 1));
	while (1) {
		mips_cache(HIT_INVALIDATE_I, addr);
		if (addr == aend)
			break;
		addr += ilsize;
	}
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	const void *addr = (const void *)(start_addr & ~(lsize - 1));
	const void *aend = (const void *)((stop - 1) & ~(lsize - 1));

	while (1) {
		mips_cache(HIT_WRITEBACK_INV_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	const void *addr = (const void *)(start_addr & ~(lsize - 1));
	const void *aend = (const void *)((stop - 1) & ~(lsize - 1));

	while (1) {
		mips_cache(HIT_INVALIDATE_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}
