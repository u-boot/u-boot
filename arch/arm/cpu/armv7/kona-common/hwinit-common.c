/*
 * Copyright 2013 Broadcom Corporation.
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#include <common.h>
#include <linux/sizes.h>

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
