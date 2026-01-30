// SPDX-License-Identifier: GPL-2.0+
//
// Copyright 2026 Fabio Estevam <festevam@nabladev.com>

#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = SZ_256M;

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	return gd->ram_top;
}
