/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

void flush_dcache_range(unsigned long start, unsigned long end)
{
}

void invalidate_icache_range(unsigned long start, unsigned long end)
{
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
