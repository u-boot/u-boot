/*
 * (C) Copyright 2014 Xilinx, Inc. Michal Simek
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */
#include <common.h>
#include <asm/arch/spl.h>

__weak void ps7_init(void)
{
	puts("Please copy ps7_init.c/h from hw project\n");
}
