// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Andes Technology Corporation
 * Randolph Lin, Andes Technology Corporation <randolph@andestech.com>
 */

#include <asm/csr.h>
#include <asm/global_data.h>
#include <asm/sbi.h>
#include <config.h>
#include <cpu_func.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <flash.h>
#include <image.h>
#include <init.h>
#include <linux/io.h>
#include <net.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	/* enable andes-l2 cache */
	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		enable_caches();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r(void)
{
	/* enable andes-l2 cache */
	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		enable_caches();

	return 0;
}
#endif

#ifdef CONFIG_SPL
void board_boot_order(u32 *spl_boot_list)
{
	u8 i;
	u32 boot_devices[] = {
#ifdef CONFIG_SPL_RAM_SUPPORT
		BOOT_DEVICE_RAM,
#endif
#ifdef CONFIG_SPL_MMC
		BOOT_DEVICE_MMC1,
#endif
	};

	for (i = 0; i < ARRAY_SIZE(boot_devices); i++)
		spl_boot_list[i] = boot_devices[i];
}
#endif
