// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <lmb.h>
#include <ram.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("RAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("Cannot get RAM size: %d\n", ret);
		return ret;
	}
	debug("RAM init base=%lx, size=%x\n", ram.base, ram.size);

	gd->ram_size = ram.size;

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	phys_addr_t reg;
	struct lmb lmb;

	/* found enough not-reserved memory to relocated U-Boot */
	lmb_init(&lmb);
	lmb_add(&lmb, gd->ram_base, gd->ram_size);
	boot_fdt_add_mem_rsv_regions(&lmb, (void *)gd->fdt_blob);
	reg = lmb_alloc(&lmb, CONFIG_SYS_MALLOC_LEN + total_size, SZ_4K);

	if (reg)
		return ALIGN(reg + CONFIG_SYS_MALLOC_LEN + total_size, SZ_4K);

	return gd->ram_top;
}
