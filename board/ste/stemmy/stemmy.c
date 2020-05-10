// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#include <common.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_init(void)
{
	return 0;
}
