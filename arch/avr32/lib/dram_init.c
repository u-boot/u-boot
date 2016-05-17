/*
 * Copyright (C) 2015 Andreas Bießmann <andreas@biessmann.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* check for the maximum amount of memory possible on AP7000 devices */
	gd->ram_size = get_ram_size(
		(void *)CONFIG_SYS_SDRAM_BASE,
		(256<<20));
	return 0;
}
