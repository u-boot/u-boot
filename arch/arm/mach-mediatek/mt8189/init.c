// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 MediaTek Inc.
 * Author: Chris-QJ Chen <chris-qj.chen@mediatek.com>
 */

#include <fdtdec.h>
#include <stdio.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/sizes.h>

#include "../cpu.h"

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

void reset_cpu(ulong addr)
{
	if (!CONFIG_IS_ENABLED(SYSRESET))
		psci_system_reset();
}

static const char *mediatek_get_segment_name_string(void)
{
	u32 seg = mediatek_sip_segment_name();

	switch (seg) {
	case 0x80:
		return "MT8391AV/AZA";
	case 0x81:
		return "MT8371AV/AZA";
	case 0x82:
		return "MT8371LV/AZA";
	case 0x88:
		return "MT8391IV/AZA";
	case 0x89:
		return "MT8371IV/AZA";
	default:
		return NULL;
	}
}

int print_cpuinfo(void)
{
	const char *seg_name = mediatek_get_segment_name_string();
	u32 part = mediatek_sip_part_name();

	if (seg_name)
		printf("CPU:   MediaTek %s\n", seg_name);
	else if (part)
		printf("CPU:   MediaTek part MT%.4x\n", part);
	else
		printf("CPU:   MediaTek MT8189\n");

	return 0;
}
