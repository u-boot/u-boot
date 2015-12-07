/*
 * Marvell PXA2xx/3xx timer driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int timer_init(void)
{
	writel(0, CONFIG_SYS_TIMER_COUNTER);
	return 0;
}
