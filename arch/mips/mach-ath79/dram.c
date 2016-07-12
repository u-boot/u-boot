/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <linux/sizes.h>
#include <asm/addrspace.h>
#include <mach/ddr.h>

phys_size_t initdram(int board_type)
{
	ddr_tap_tuning();
	return get_ram_size((void *)KSEG1, SZ_256M);
}
