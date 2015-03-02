/*
 * Copyright (C) 2013-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/compiler.h>
#include <mach/led.h>
#include <mach/board.h>

void __weak bcu_init(void)
{
};
void sbc_init(void);
void sg_init(void);
void pll_init(void);
void pin_init(void);
void memconf_init(void);
void early_clkrst_init(void);
int umc_init(void);
void enable_dpll_ssc(void);

void spl_board_init(void)
{
	bcu_init();

	sbc_init();

	sg_init();

	uniphier_board_reset();

	pll_init();

	uniphier_board_init();

	led_write(L, 0, , );

	memconf_init();

	led_write(L, 1, , );

	early_clkrst_init();

	led_write(L, 2, , );

	{
		int res;

		res = umc_init();
		if (res < 0) {
			while (1)
				;
		}
	}
	led_write(L, 3, , );

	enable_dpll_ssc();

	led_write(L, 4, , );
}
