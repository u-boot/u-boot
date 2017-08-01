/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <netdev.h>

int arch_cpu_init(void)
{
	return 0;
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return default_print_cpuinfo();
}

void board_final_cleanup(void)
{
}

int misc_init_r(void)
{
	return 0;
}
