/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * based on arch/arm/cpu/armv7/omap3/cache.S
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.align 5

#include <linux/linkage.h>

#ifndef CONFIG_SYS_L2CACHE_OFF
ENTRY(v7_outer_cache_enable)
	push	{r0, r1, r2, lr}
	mrc	15, 0, r3, cr1, cr0, 1
	orr	r3, r3, #2
	mcr	15, 0, r3, cr1, cr0, 1
	pop	{r1, r2, r3, pc}
ENDPROC(v7_outer_cache_enable)

ENTRY(v7_outer_cache_disable)
	push	{r0, r1, r2, lr}
	mrc	15, 0, r3, cr1, cr0, 1
	bic	r3, r3, #2
	mcr	15, 0, r3, cr1, cr0, 1
	pop	{r1, r2, r3, pc}
ENDPROC(v7_outer_cache_disable)
#endif
