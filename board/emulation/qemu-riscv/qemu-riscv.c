// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <fdtdec.h>

#define MROM_FDT_ADDR	0x1020

int board_init(void)
{
	return 0;
}

void *board_fdt_blob_setup(void)
{
	/*
	 * QEMU loads a generated DTB for us immediately
	 * after the reset vectors in the MROM
	 */
	return (void *)MROM_FDT_ADDR;
}
