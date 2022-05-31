// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/asm.h>
#include <asm/cache.h>

static void __invalidate_icache(ulong addr, ulong size)
{
	if (CONFIG_IS_ENABLED(XILINX_MICROBLAZE0_USE_WIC)) {
		for (int i = 0; i < size; i += 4) {
			asm volatile (
				"wic	%0, r0;"
				"nop;"
				:
				: "r" (addr + i)
				: "memory");
		}
	}
}

void invalidate_icache_all(void)
{
	__invalidate_icache(0, CONFIG_XILINX_MICROBLAZE0_ICACHE_SIZE);
}

static void __flush_dcache(ulong addr, ulong size)
{
	if (CONFIG_IS_ENABLED(XILINX_MICROBLAZE0_USE_WDC)) {
		for (int i = 0; i < size; i += 4) {
			asm volatile (
				"wdc.flush	%0, r0;"
				"nop;"
				:
				: "r" (addr + i)
				: "memory");
		}
	}
}

void flush_dcache_all(void)
{
	__flush_dcache(0, CONFIG_XILINX_MICROBLAZE0_DCACHE_SIZE);
}

int dcache_status(void)
{
	int i = 0;
	int mask = 0x80;
	__asm__ __volatile__ ("mfs %0,rmsr"::"r" (i):"memory");
	/* i&=0x80 */
	__asm__ __volatile__ ("and %0,%0,%1"::"r" (i), "r" (mask):"memory");
	return i;
}

int icache_status(void)
{
	int i = 0;
	int mask = 0x20;
	__asm__ __volatile__ ("mfs %0,rmsr"::"r" (i):"memory");
	/* i&=0x20 */
	__asm__ __volatile__ ("and %0,%0,%1"::"r" (i), "r" (mask):"memory");
	return i;
}

void icache_enable(void)
{
	MSRSET(0x20);
}

void icache_disable(void)
{
	invalidate_icache_all();

	MSRCLR(0x20);
}

void dcache_enable(void)
{
	MSRSET(0x80);
}

void dcache_disable(void)
{
	flush_dcache_all();

	MSRCLR(0x80);
}

void flush_cache(ulong addr, ulong size)
{
	__invalidate_icache(addr, size);
	__flush_dcache(addr, size);
}

void flush_cache_all(void)
{
	invalidate_icache_all();
	flush_dcache_all();
}
