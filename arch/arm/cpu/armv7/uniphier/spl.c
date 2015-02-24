/*
 * Copyright (C) 2013-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/compiler.h>
#include <asm/arch/led.h>
#include <asm/arch/board.h>

void __weak bcu_init(void)
{
};
void sbc_init(void);
void sg_init(void);
void pll_init(void);
void pin_init(void);
void clkrst_init(void);
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

	clkrst_init();

	led_write(L, 1, , );

	{
		int res;

		res = umc_init();
		if (res < 0) {
			while (1)
				;
		}
	}
	led_write(L, 2, , );

	enable_dpll_ssc();

	led_write(L, 3, , );
}
