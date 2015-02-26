/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/led.h>

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	led_write(U, B, O, O);

	return 0;
}

#if CONFIG_NR_DRAM_BANKS >= 2
void dram_init_banksize(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = CONFIG_SDRAM0_BASE;
	gd->bd->bi_dram[0].size  = CONFIG_SDRAM0_SIZE;
	gd->bd->bi_dram[1].start = CONFIG_SDRAM1_BASE;
	gd->bd->bi_dram[1].size  = CONFIG_SDRAM1_SIZE;
}
#endif
