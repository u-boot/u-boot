// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/rcar-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <dm.h>
#include <fdt_support.h>
#include <hang.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/uclass-internal.h>
#include <asm/arch/renesas.h>
#include <linux/libfdt.h>

#ifdef CONFIG_RCAR_64

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

int __weak board_init(void)
{
	return 0;
}

#endif
