// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 */

#include <common.h>
#include <linux/libfdt.h>
#include <linux/err.h>
#include <asm/arch/gx.h>
#include <asm/arch/sm.h>
#include <asm/armv8/mmu.h>
#include <asm/unaligned.h>
#include <linux/sizes.h>
#include <efi_loader.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	const fdt64_t *val;
	int offset;
	int len;

	offset = fdt_path_offset(gd->fdt_blob, "/memory");
	if (offset < 0)
		return -EINVAL;

	val = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (len < sizeof(*val) * 2)
		return -EINVAL;

	/* Use unaligned access since cache is still disabled */
	gd->ram_size = get_unaligned_be64(&val[1]);

	return 0;
}

phys_size_t get_effective_memsize(void)
{
	/* Size is reported in MiB, convert it in bytes */
	return ((readl(GX_AO_SEC_GP_CFG0) & GX_AO_MEM_SIZE_MASK)
			>> GX_AO_MEM_SIZE_SHIFT) * SZ_1M;
}

static void meson_board_add_reserved_memory(void *fdt, u64 start, u64 size)
{
	int ret;

	ret = fdt_add_mem_rsv(fdt, start, size);
	if (ret)
		printf("Could not reserve zone @ 0x%llx\n", start);

	if (IS_ENABLED(CONFIG_EFI_LOADER)) {
		efi_add_memory_map(start,
				   ALIGN(size, EFI_PAGE_SIZE) >> EFI_PAGE_SHIFT,
				   EFI_RESERVED_MEMORY_TYPE, false);
	}
}

void meson_gx_init_reserved_memory(void *fdt)
{
	u64 bl31_size, bl31_start;
	u64 bl32_size, bl32_start;
	u32 reg;

	/*
	 * Get ARM Trusted Firmware reserved memory zones in :
	 * - AO_SEC_GP_CFG3: bl32 & bl31 size in KiB, can be 0
	 * - AO_SEC_GP_CFG5: bl31 physical start address, can be NULL
	 * - AO_SEC_GP_CFG4: bl32 physical start address, can be NULL
	 */

	reg = readl(GX_AO_SEC_GP_CFG3);

	bl31_size = ((reg & GX_AO_BL31_RSVMEM_SIZE_MASK)
			>> GX_AO_BL31_RSVMEM_SIZE_SHIFT) * SZ_1K;
	bl32_size = (reg & GX_AO_BL32_RSVMEM_SIZE_MASK) * SZ_1K;

	bl31_start = readl(GX_AO_SEC_GP_CFG5);
	bl32_start = readl(GX_AO_SEC_GP_CFG4);

	/*
	 * Early Meson GX Firmware revisions did not provide the reserved
	 * memory zones in the registers, keep fixed memory zone handling.
	 */
	if (IS_ENABLED(CONFIG_MESON_GX) &&
	    !reg && !bl31_start && !bl32_start) {
		bl31_start = 0x10000000;
		bl31_size = 0x200000;
	}

	/* Add first 16MiB reserved zone */
	meson_board_add_reserved_memory(fdt, 0, GX_FIRMWARE_MEM_SIZE);

	/* Add BL31 reserved zone */
	if (bl31_start && bl31_size)
		meson_board_add_reserved_memory(fdt, bl31_start, bl31_size);

	/* Add BL32 reserved zone */
	if (bl32_start && bl32_size)
		meson_board_add_reserved_memory(fdt, bl32_start, bl32_size);
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
}

static struct mm_region gx_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xc0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xc0000000UL,
		.phys = 0xc0000000UL,
		.size = 0x30000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = gx_mem_map;
