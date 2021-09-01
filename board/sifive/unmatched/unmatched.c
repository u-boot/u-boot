// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021, SiFive Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/sections.h>

void *board_fdt_blob_setup(void)
{
	if (IS_ENABLED(CONFIG_OF_SEPARATE)) {
		if (gd->arch.firmware_fdt_addr)
			return (ulong *)gd->arch.firmware_fdt_addr;
		else
			return (ulong *)&_end;
	}
}

int board_init(void)
{
	/* enable all cache ways */
	enable_caches();

	return 0;
}
