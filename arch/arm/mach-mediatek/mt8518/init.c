// SPDX-License-Identifier: GPL-2.0
/*
 * Configuration for MediaTek MT8518 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <ram.h>
#include <asm/arch/misc.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/mt8518-clk.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

void reset_cpu(void)
{
	psci_system_reset();
}

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT8518\n");
	return 0;
}
