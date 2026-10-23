// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 BayLibre, SAS
 * Copyright (C) 2026 MediaTek Inc.
 * Author: Chris Chen <chris-qj.chen@mediatek.com>
 */

#include <fdtdec.h>
#include <stdio.h>
#include <wdt.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <dm/uclass.h>
#include <linux/kernel.h>
#include <linux/sizes.h>

#include "../cpu.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/*
	 * Limit gd->ram_top not exceeding SZ_4G. Because some peripherals like
	 * MMC requires DMA buffer allocated below SZ_4G.
	 */
	return min(gd->ram_top, SZ_4G);
}

void reset_cpu(void)
{
	if (CONFIG_IS_ENABLED(PSCI_RESET)) {
		psci_system_reset();
	} else {
		struct udevice *wdt;

		uclass_first_device(UCLASS_WDT, &wdt);
		if (wdt)
			wdt_expire_now(wdt, 0);
	}
}

int print_cpuinfo(void)
{
	u32 part = mediatek_sip_part_name();

	if (part)
		printf("CPU:   MediaTek MT%.4x\n", part);
	else
		printf("CPU:   MediaTek MT8366 series\n");

	return 0;
}
