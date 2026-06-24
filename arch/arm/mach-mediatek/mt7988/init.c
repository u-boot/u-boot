// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <fdtdec.h>
#include <init.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base();
	if (ret)
		return ret;

	gd->ram_size = get_ram_size((void *)gd->ram_base, SZ_8G);

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/*
	 * Limit gd->ram_top not exceeding SZ_4G. Because some peripherals like
	 * MMC requires DMA buffer allocated below SZ_4G.
	 */
	return min(gd->ram_top, SZ_4G);
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
}
