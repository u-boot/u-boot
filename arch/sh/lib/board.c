/*
 * Copyright (C) 2016 Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
