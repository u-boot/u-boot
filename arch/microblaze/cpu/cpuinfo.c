// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */
#include <common.h>
#include <asm/cpuinfo.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void microblaze_early_cpuinfo_init(void)
{
	struct microblaze_cpuinfo *ci = gd_cpuinfo();

	ci->icache_size = CONFIG_XILINX_MICROBLAZE0_ICACHE_SIZE;
	ci->icache_line_length = 4;

	ci->dcache_size = CONFIG_XILINX_MICROBLAZE0_DCACHE_SIZE;
	ci->dcache_line_length = 4;
}
