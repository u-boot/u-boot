// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc.
 * Copyright (C) 2025 BayLibre, SAS
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Chris-QJ Chen <chris-qj.chen@mediatek.com>
 */

#include <asm/global_data.h>
#include <asm/system.h>
#include <dm/uclass.h>
#include <linux/sizes.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	fdtdec_setup_mem_size_base();

	/*
	 * Limit gd->ram_top not exceeding SZ_4G.  Some periphals like mmc
	 * requires DMA buffer allocated below SZ_4G.
	 *
	 * Note: SZ_1M is for adjusting gd->relocaddr, the reserved memory for
	 * u-boot itself.
	 */
	if (gd->ram_base + gd->ram_size >= SZ_4G)
		gd->mon_len = (gd->ram_base + gd->ram_size + SZ_1M) - SZ_4G;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
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
	printf("CPU:   MediaTek MT8188\n");
	return 0;
}
