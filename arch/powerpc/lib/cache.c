// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <cpu_func.h>
#include <stdio.h>
#include <asm/cache.h>
#include <watchdog.h>

static ulong maybe_watchdog_reset(ulong flushed)
{
	flushed += CONFIG_SYS_CACHELINE_SIZE;
	if (flushed >= CONFIG_CACHE_FLUSH_WATCHDOG_THRESHOLD) {
		schedule();
		flushed = 0;
	}
	return flushed;
}

void flush_cache(ulong start_addr, ulong size)
{
	ulong addr, start, end;
	ulong flushed = 0;

	start = start_addr & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
	end = start_addr + size - 1;

	for (addr = start; (addr <= end) && (addr >= start);
			addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile("dcbst 0,%0" : : "r" (addr) : "memory");
		flushed = maybe_watchdog_reset(flushed);
	}
	/* wait for all dcbst to complete on bus */
	asm volatile("sync" : : : "memory");

	for (addr = start; (addr <= end) && (addr >= start);
			addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile("icbi 0,%0" : : "r" (addr) : "memory");
		flushed = maybe_watchdog_reset(flushed);
	}
	asm volatile("sync" : : : "memory");
	/* flush prefetch queue */
	asm volatile("isync" : : : "memory");
}

/*
 * Default implementation:
 * do a range flush for the entire range
 */
void flush_dcache_all(void)
{
	flush_dcache_range(0, ~0);
}

void invalidate_icache_all(void)
{
	puts("No arch specific invalidate_icache_all available!\n");
}
