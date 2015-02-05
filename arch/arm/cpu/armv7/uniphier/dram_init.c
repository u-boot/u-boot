/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}
