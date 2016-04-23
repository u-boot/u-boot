/*
 * Copyright (c) 2010 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>

void reset_cpu(ulong addr)
{
#ifdef CONFIG_CPU_V7
	writel(0x1, samsung_get_base_swreset());
#endif
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#ifdef CONFIG_ARM64
void lowlevel_init(void)
{
	armv8_switch_to_el2();
	armv8_switch_to_el1();
}
#endif
