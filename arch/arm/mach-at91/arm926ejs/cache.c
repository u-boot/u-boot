/*
 * (C) Copyright 2016
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <linux/types.h>
#include <common.h>

void enable_caches(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
#endif
}

#ifndef CONFIG_SYS_ICACHE_OFF
/* Invalidate entire I-cache and branch predictor array */
void invalidate_icache_all(void)
{
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c5, 0" : : "r" (i));
}
#else
void invalidate_icache_all(void)
{
}
#endif
