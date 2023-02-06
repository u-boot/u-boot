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
}

void icache_disable(void)
{
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

int dcache_status(void)
{
	return 0;
}
