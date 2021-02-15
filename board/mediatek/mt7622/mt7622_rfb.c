// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <common.h>
#include <config.h>
#include <env.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	return 0;
}

int board_late_init(void)
{
	gd->env_valid = 1; //to load environment variable from persistent store
	env_relocate();
	return 0;
}
