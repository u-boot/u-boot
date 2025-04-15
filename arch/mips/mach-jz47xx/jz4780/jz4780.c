// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4780 common routines
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <config.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <mach/jz4780.h>
#include <mach/jz4780_dram.h>
#include <mmc.h>
#include <spl.h>

#ifdef CONFIG_XPL_BUILD
/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __section(".bss");

void board_init_f(ulong dummy)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);
	struct mmc *mmc;
	unsigned long count;
	struct legacy_img_hdr *header;
	int ret;

	/* Set global data pointer */
	gd = &gdata;

	timer_init();
	pll_init();
	sdram_init();
	enable_caches();

	/* Clear the BSS */
	memset(__bss_start, 0, (size_t)__bss_end - (size_t)__bss_start);

	gd->flags |= GD_FLG_SPL_INIT;

	ret = mmc_initialize(NULL);
	if (ret)
		hang();

	mmc = find_mmc_device(BOOT_DEVICE_MMC1);
	if (ret)
		hang();

	ret = mmc_init(mmc);
	if (ret)
		hang();

	header = (struct legacy_img_hdr *)(CONFIG_TEXT_BASE -
					 sizeof(struct legacy_img_hdr));

	count = blk_dread(mmc_get_blk_desc(mmc),
			  CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR,
		0x800, header);
	if (count == 0)
		hang();

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)CONFIG_TEXT_BASE;

	image_entry();

	hang();
}
#endif /* CONFIG_XPL_BUILD */

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	return CFG_SYS_SDRAM_BASE + (256 * 1024 * 1024);
}

int print_cpuinfo(void)
{
	printf("CPU:   Ingenic JZ4780\n");
	return 0;
}
