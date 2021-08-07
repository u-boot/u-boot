// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

void clock_init(void)
{
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	return 0;
}

u32 get_lpuart_clk(void)
{
	return 24000000;
}
