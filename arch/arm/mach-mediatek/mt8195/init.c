// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 MediaTek Inc.
 * Copyright (C) 2026 BayLibre, SAS
 * Author: Julien Stephan <jstephan@baylibre.com>
 *         Chris-QJ Chen <chris-qj.chen@mediatek.com>
 */

#include <asm/armv8/mmu.h>
#include <asm/system.h>
#include <dm/uclass.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

phys_size_t get_effective_memsize(void)
{
	/*
	 * Limit gd->ram_top not exceeding SZ_4G. Because some peripherals like
	 * MMC requires DMA buffer allocated below SZ_4G.
	 */
	return min(SZ_4G - gd->ram_base, gd->ram_size);
}

int mtk_soc_early_init(void)
{
	return 0;
}

void reset_cpu(void)
{
	struct udevice *wdt;

	if (IS_ENABLED(CONFIG_PSCI_RESET)) {
		psci_system_reset();
	} else {
		uclass_first_device(UCLASS_WDT, &wdt);
		if (wdt)
			wdt_expire_now(wdt, 0);
	}
}

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT8195\n");
	return 0;
}
