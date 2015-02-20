/*
 * Copyright (c) 2010 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>

enum l2_cache_params {
	CACHE_TAG_RAM_SETUP = (1 << 9),
	CACHE_DATA_RAM_SETUP = (1 << 5),
	CACHE_TAG_RAM_LATENCY = (2 << 6),
	CACHE_DATA_RAM_LATENCY = (2 << 0),
	CACHE_ENABLE_CLEAN_EVICT = (0 << 3),
	CACHE_DISABLE_CLEAN_EVICT = (1 << 3)
};

void reset_cpu(ulong addr)
{
	writel(0x1, samsung_get_base_swreset());
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#ifndef CONFIG_SYS_L2CACHE_OFF
/*
 * Set L2 cache parameters
 */
static void exynos5_set_l2cache_params(void)
{
	unsigned int val = 0;

	/* Read L2CTLR value */
	asm volatile("mrc p15, 1, %0, c9, c0, 2\n" : "=r"(val));

	/* Set cache setup and latency cycles */
	val |= CACHE_TAG_RAM_SETUP |
		CACHE_DATA_RAM_SETUP |
		CACHE_TAG_RAM_LATENCY |
		CACHE_DATA_RAM_LATENCY;

	/* Write new vlaue to L2CTLR */
	asm volatile("mcr p15, 1, %0, c9, c0, 2\n" : : "r"(val));

	if (proid_is_exynos5420() || proid_is_exynos5800()) {
		/* Read L2ACTLR value */
		asm volatile("mrc	p15, 1, %0, c15, c0, 0" : "=r" (val));

		/* Disable clean/evict push to external */
		val |= CACHE_DISABLE_CLEAN_EVICT;

		/* Write new vlaue to L2ACTLR */
		asm volatile("mcr	p15, 1, %0, c15, c0, 0" : : "r" (val));
	}
}

/*
 * Sets L2 cache related parameters before enabling data cache
 */
void v7_outer_cache_enable(void)
{
	if (cpu_is_exynos5())
		exynos5_set_l2cache_params();
}
#endif
