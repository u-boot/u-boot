// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <fdtdec.h>
#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/system.h>
#include <asm/global_data.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT7622\n");
	return 0;
}

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base();
	if (ret)
		return ret;

	gd->ram_size = get_ram_size((void *)gd->ram_base, SZ_1G);

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}
