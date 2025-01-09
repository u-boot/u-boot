// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <dm.h>
#include <efi_loader.h>
#include <image.h>
#include <init.h>
#include <lmb.h>
#include <log.h>
#include <ram.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <mach/stm32mp.h>

DECLARE_GLOBAL_DATA_PTR;

int optee_get_reserved_memory(u32 *start, u32 *size)
{
	fdt_addr_t fdt_mem_size;
	fdt_addr_t fdt_start;
	ofnode node;

	node = ofnode_path("/reserved-memory/optee");
	if (!ofnode_valid(node)) {
		node = ofnode_path("/reserved-memory/optee_core");
		if (!ofnode_valid(node))
			return -ENOENT;
	}

	fdt_start = ofnode_get_addr_size(node, "reg", &fdt_mem_size);
	*start = fdt_start;
	*size = fdt_mem_size;
	return (fdt_start < 0) ? fdt_start : 0;
}

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	/* in case there is no RAM driver, retrieve DDR size from DT */
	if (ret == -ENODEV) {
		return fdtdec_setup_mem_size_base();
	} else if (ret) {
		log_err("RAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		log_debug("Cannot get RAM size: %d\n", ret);
		return ret;
	}
	log_debug("RAM init base=%p, size=%zx\n", (void *)ram.base, ram.size);

	gd->ram_size = ram.size;

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	phys_size_t size;
	phys_addr_t reg;
	u32 optee_start, optee_size;

	if (!total_size)
		return gd->ram_top;

	/*
	 * make sure U-Boot uses address space below 4GB boundaries even
	 * if the effective available memory is bigger
	 */
	gd->ram_top = clamp_val(gd->ram_top, 0, SZ_4G - 1);

	/* add 8M for U-Boot reserved memory: display, fdt, gd,... */
	size = ALIGN(SZ_8M + CONFIG_SYS_MALLOC_LEN + total_size, MMU_SECTION_SIZE);

	reg = ALIGN(gd->ram_top - size, MMU_SECTION_SIZE);

	/* Reserved memory for OP-TEE at END of DDR for STM32MP1 SoC */
	if (IS_ENABLED(CONFIG_STM32MP13X) || IS_ENABLED(CONFIG_STM32MP15X)) {
		if (!optee_get_reserved_memory(&optee_start, &optee_size))
			reg = ALIGN(optee_start - size, MMU_SECTION_SIZE);
	}

	/* before relocation, mark the U-Boot memory as cacheable by default */
	if (!(gd->flags & GD_FLG_RELOC))
		mmu_set_region_dcache_behaviour(reg, size, DCACHE_DEFAULT_OPTION);

	return reg + size;
}
