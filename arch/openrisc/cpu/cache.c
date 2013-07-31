/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/system.h>

void flush_dcache_range(unsigned long addr, unsigned long stop)
{
	ulong block_size = (mfspr(SPR_DCCFGR) & SPR_DCCFGR_CBS) ? 32 : 16;

	while (addr < stop) {
		mtspr(SPR_DCBFR, addr);
		addr += block_size;
	}
}

void invalidate_dcache_range(unsigned long addr, unsigned long stop)
{
	ulong block_size = (mfspr(SPR_DCCFGR) & SPR_DCCFGR_CBS) ? 32 : 16;

	while (addr < stop) {
		mtspr(SPR_DCBIR, addr);
		addr += block_size;
	}
}

static void invalidate_icache_range(unsigned long addr, unsigned long stop)
{
	ulong block_size = (mfspr(SPR_ICCFGR) & SPR_ICCFGR_CBS) ? 32 : 16;

	while (addr < stop) {
		mtspr(SPR_ICBIR, addr);
		addr += block_size;
	}
}

void flush_cache(unsigned long addr, unsigned long size)
{
	flush_dcache_range(addr, addr + size);
	invalidate_icache_range(addr, addr + size);
}

int icache_status(void)
{
	return mfspr(SPR_SR) & SPR_SR_ICE;
}

int checkicache(void)
{
	unsigned long iccfgr;
	unsigned long cache_set_size;
	unsigned long cache_ways;
	unsigned long cache_block_size;

	iccfgr = mfspr(SPR_ICCFGR);
	cache_ways = 1 << (iccfgr & SPR_ICCFGR_NCW);
	cache_set_size = 1 << ((iccfgr & SPR_ICCFGR_NCS) >> 3);
	cache_block_size = (iccfgr & SPR_ICCFGR_CBS) ? 32 : 16;

	return cache_set_size * cache_ways * cache_block_size;
}

int dcache_status(void)
{
	return mfspr(SPR_SR) & SPR_SR_DCE;
}

int checkdcache(void)
{
	unsigned long dccfgr;
	unsigned long cache_set_size;
	unsigned long cache_ways;
	unsigned long cache_block_size;

	dccfgr = mfspr(SPR_DCCFGR);
	cache_ways = 1 << (dccfgr & SPR_DCCFGR_NCW);
	cache_set_size = 1 << ((dccfgr & SPR_DCCFGR_NCS) >> 3);
	cache_block_size = (dccfgr & SPR_DCCFGR_CBS) ? 32 : 16;

	return cache_set_size * cache_ways * cache_block_size;
}

void dcache_enable(void)
{
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_DCE);
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
}

void dcache_disable(void)
{
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_DCE);
}

void icache_enable(void)
{
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_ICE);
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
	asm volatile("l.nop");
}

void icache_disable(void)
{
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_ICE);
}

int cache_init(void)
{
	if (mfspr(SPR_UPR) & SPR_UPR_ICP) {
		icache_disable();
		invalidate_icache_range(0, checkicache());
		icache_enable();
	}

	if (mfspr(SPR_UPR) & SPR_UPR_DCP) {
		dcache_disable();
		invalidate_dcache_range(0, checkdcache());
		dcache_enable();
	}

	return 0;
}
