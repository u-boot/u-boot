// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/condor/condor.c
 *     This file is Condor board support.
 *
 * Copyright (C) 2019 Marek Vasut <marek.vasut+renesas@gmail.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}
