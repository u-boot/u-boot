/*
 * Copyright (C) 2013-2015 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/cache.h>
#include <common.h>

int init_cache_f_r(void)
{
	sync_n_cleanup_cache_all();

	return 0;
}
