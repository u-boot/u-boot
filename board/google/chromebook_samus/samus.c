/*
 * Copyright (C) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/cpu.h>

int arch_early_init_r(void)
{
	return cpu_run_reference_code();
}

int board_early_init_f(void)
{
	return 0;
}
