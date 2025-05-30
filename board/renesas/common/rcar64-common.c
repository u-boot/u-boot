// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/common/common.c
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
#include <asm/system.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int ret = fdtdec_setup_mem_size_base();

	if (current_el() == 3 && gd->ram_base == 0x48000000) {
		/*
		 * If this U-Boot runs in EL3, make the bottom 128 MiB
		 * available for loading of follow up firmware blobs.
		 */
		gd->ram_base -= 0x8000000;
		gd->ram_size += 0x8000000;
	}

	return ret;
}

__weak void renesas_dram_init_banksize(void) { }

int dram_init_banksize(void)
{
	int bank;

	fdtdec_setup_memory_banksize();

	if (current_el() != 3)
		return 0;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (gd->bd->bi_dram[bank].start != 0x48000000)
			continue;

		/*
		 * If this U-Boot runs in EL3, make the bottom 128 MiB
		 * available for loading of follow up firmware blobs.
		 */
		gd->bd->bi_dram[bank].start -= 0x8000000;
		gd->bd->bi_dram[bank].size += 0x8000000;
		break;
	}

	renesas_dram_init_banksize();

	return 0;
}

int __weak board_init(void)
{
	return 0;
}

int __weak board_early_init_f(void)
{
	return 0;
}
