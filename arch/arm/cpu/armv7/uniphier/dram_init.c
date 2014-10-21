/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/led.h>

int umc_init(void);
void enable_dpll_ssc(void);

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	led_write(B, 4, , );

	{
		int res;

		res = umc_init();
		if (res < 0)
			return res;
	}
	led_write(B, 5, , );

	enable_dpll_ssc();
#endif

	led_write(B, 6, , );

	return 0;
}
