// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for QCS404-EVB
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <fdt_support.h>
#include <asm/arch/dram.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int board_init(void)
{
	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}
