// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <dm.h>
#include <image.h>
#include <init.h>
#include <lmb.h>
#include <log.h>
#include <ram.h>
#include <asm/global_data.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		log_debug("RAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		log_debug("Cannot get RAM size: %d\n", ret);
		return ret;
	}
	log_debug("RAM init base=%lx, size=%x\n", ram.base, ram.size);

	gd->ram_size = ram.size;

	return 0;
}

phys_size_t board_get_usable_ram_top(phys_size_t total_size)
{
	phys_size_t size;
	phys_addr_t reg;
	struct lmb lmb;

	if (!total_size)
		return gd->ram_top;

	/* found enough not-reserved memory to relocated U-Boot */
	lmb_init(&lmb);
	lmb_add(&lmb, gd->ram_base, gd->ram_size);
	boot_fdt_add_mem_rsv_regions(&lmb, (void *)gd->fdt_blob);
	/* add 8M for reserved memory for display, fdt, gd,... */
	size = ALIGN(SZ_8M + CONFIG_SYS_MALLOC_LEN + total_size, MMU_SECTION_SIZE),
	reg = lmb_alloc(&lmb, size, MMU_SECTION_SIZE);

	if (!reg)
		reg = gd->ram_top - size;

	/* before relocation, mark the U-Boot memory as cacheable by default */
	if (!(gd->flags & GD_FLG_RELOC))
		mmu_set_region_dcache_behaviour(reg, size, DCACHE_DEFAULT_OPTION);

	return reg + size;
}
