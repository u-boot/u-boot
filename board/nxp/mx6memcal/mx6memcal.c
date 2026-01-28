// SPDX-License-Identifier: GPL-2.0+
/*
 * mx6memcal board support - provides a minimal, UART-only
 * U-Boot that's capable of running a memory test.
 *
 * Copyright (C) 2016 Nelson Integration, LLC
 * Author: Eric Nelson <eric@nelint.com>
 */

#include <init.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: mx6memcal\n");
	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}
