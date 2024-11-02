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

int board_fdt_blob_setup(void **fdtp)
{
	if (gd->arch.firmware_fdt_addr) {
		*fdtp = (ulong *)(uintptr_t)gd->arch.firmware_fdt_addr;
		return 0;
	}

	return -EEXIST;
}

int board_init(void)
{
	/* enable all cache ways */
	enable_caches();

	return 0;
}
