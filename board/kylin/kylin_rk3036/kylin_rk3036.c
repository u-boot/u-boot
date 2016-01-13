/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/sdram_rk3036.h>

DECLARE_GLOBAL_DATA_PTR;

void get_ddr_config(struct rk3036_ddr_config *config)
{
	/* K4B4G1646Q config */
	config->ddr_type = 3;
	config->rank = 1;
	config->cs0_row = 15;
	config->cs1_row = 15;

	/* 8bank */
	config->bank = 3;
	config->col = 10;

	/* 16bit bw */
	config->bw = 1;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = sdram_size();

	return 0;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
