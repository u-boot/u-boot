// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021, SiFive Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <cpu_func.h>
#include <dm.h>
#include <asm/sections.h>

int board_init(void)
{
	/* enable all cache ways */
	enable_caches();

	return 0;
}
