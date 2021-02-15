// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	/* We do some SoC one time setting here. */

	return 0;
}
