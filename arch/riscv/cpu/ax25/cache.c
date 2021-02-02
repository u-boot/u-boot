// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/cache.h>
#include <dm/uclass-internal.h>
#include <cache.h>
#include <asm/csr.h>

#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
/* mcctlcommand */
#define CCTL_REG_MCCTLCOMMAND_NUM	0x7cc

/* D-cache operation */
#define CCTL_L1D_WBINVAL_ALL	6
#endif
#endif

#ifdef CONFIG_V5L2_CACHE
static void _cache_enable(void)
{
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CACHE, &dev);

	if (dev)
		cache_enable(dev);
}

static void _cache_disable(void)
{
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CACHE, &dev);

	if (dev)
		cache_disable(dev);
}
#endif

void flush_dcache_all(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
#endif
#endif
#endif
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_all();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_all();
}

void icache_enable(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
#endif
}

void icache_disable(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"fence.i\n\t"
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x1\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#endif
#endif
}

void dcache_enable(void)
{
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"ori t0, t1, 0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#ifdef CONFIG_V5L2_CACHE
	_cache_enable();
#endif
#endif
#endif
}

void dcache_disable(void)
{
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	csr_write(CCTL_REG_MCCTLCOMMAND_NUM, CCTL_L1D_WBINVAL_ALL);
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi t0, t1, ~0x2\n\t"
		"csrw mcache_ctl, t0\n\t"
	);
#endif
#ifdef CONFIG_V5L2_CACHE
	_cache_disable();
#endif
#endif
#endif
}

int icache_status(void)
{
	int ret = 0;

#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x01\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif
#endif

	return ret;
}

int dcache_status(void)
{
	int ret = 0;

#ifdef CONFIG_RISCV_NDS_CACHE
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	asm volatile (
		"csrr t1, mcache_ctl\n\t"
		"andi	%0, t1, 0x02\n\t"
		: "=r" (ret)
		:
		: "memory"
	);
#endif
#endif

	return ret;
}
