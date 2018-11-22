// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>

void flush_dcache_range(unsigned long start, unsigned long end)
{
}

void invalidate_icache_range(unsigned long start, unsigned long end)
{
	/*
	 * RISC-V does not have an instruction for invalidating parts of the
	 * instruction cache. Invalidate all of it instead.
	 */
	invalidate_icache_all();
}

void invalidate_icache_all(void)
{
	asm volatile ("fence.i" ::: "memory");
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
}

void flush_cache(unsigned long addr, unsigned long size)
{
}

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

int dcache_status(void)
{
	return 0;
}
