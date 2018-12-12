// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>

void icache_enable(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

void icache_disable(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"fence.i\n\t"
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

void dcache_enable(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

void dcache_disable(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"fence\n\t"
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
}

int icache_status(void)
{
	int ret = 0;

#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif

	return ret;
}

int dcache_status(void)
{
	int ret = 0;

#ifdef CONFIG_RISCV_NDS_CACHE
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif

	return ret;
}
