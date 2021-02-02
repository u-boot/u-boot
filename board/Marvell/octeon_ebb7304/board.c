// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <ram.h>

#include <mach/octeon_ddr.h>

#include "board_ddr.h"

#define EBB7304_DEF_DRAM_FREQ	800

static struct ddr_conf board_ddr_conf[] = {
	 OCTEON_EBB7304_DDR_CONFIGURATION
};

struct ddr_conf *octeon_ddr_conf_table_get(int *count, int *def_ddr_freq)
{
	*count = ARRAY_SIZE(board_ddr_conf);
	*def_ddr_freq = EBB7304_DEF_DRAM_FREQ;

	return board_ddr_conf;
}
