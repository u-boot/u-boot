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
	/* we are not generate ICACHE size -> flush whole cache */
	flush_cache(0, 32768);
	MSRCLR(0x20);
}

void dcache_enable(void)
{
	MSRSET(0x80);
}

void dcache_disable(void)
{
	flush_cache(0, XILINX_DCACHE_BYTE_SIZE);

	MSRCLR(0x80);
}

void flush_cache(ulong addr, ulong size)
{
	int i;
	for (i = 0; i < size; i += 4) {
		if (CONFIG_IS_ENABLED(XILINX_MICROBLAZE0_USE_WIC)) {
			asm volatile (
				"wic	%0, r0;"
				"nop;"
				:
				: "r" (addr + i)
				: "memory");
		}

		if (CONFIG_IS_ENABLED(XILINX_MICROBLAZE0_USE_WDC)) {
			asm volatile (
				"wdc.flush	%0, r0;"
				"nop;"
				:
				: "r" (addr + i)
				: "memory");
		}
	}
}
