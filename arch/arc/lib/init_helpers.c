/*
 * Copyright (C) 2013-2015 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int init_cache_f_r(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
	/* Make sure no stale entries persist from before we disabled cache */
	invalidate_icache_all();
#endif

#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
	/* Make sure no stale entries persist from before we disabled cache */
	invalidate_dcache_all();
#endif
	return 0;
}
