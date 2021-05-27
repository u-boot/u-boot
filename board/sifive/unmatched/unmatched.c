// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021, SiFive Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/arch/cache.h>

int board_init(void)
{
	int ret;

	/* enable all cache ways */
	ret = cache_enable_ways();
	if (ret) {
		debug("%s: could not enable cache ways\n", __func__);
		return ret;
	}
	return 0;
}
