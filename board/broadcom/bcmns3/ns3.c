// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom.
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-bcmns3/bl33_info.h>

/* Default reset-level = 3 and strap-val = 0 */
#define L3_RESET	30

static struct mm_region ns3_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = ns3_mem_map;

DECLARE_GLOBAL_DATA_PTR;

/*
 * Force the bl33_info to the data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
struct bl33_info *bl33_info __section(".data");

int board_init(void)
{
	if (bl33_info->version != BL33_INFO_VERSION)
		printf("*** warning: ATF BL31 and U-Boot not in sync! ***\n");

	return 0;
}

int board_late_init(void)
{
	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

void reset_cpu(ulong level)
{
	u32 reset_level, strap_val;

	/* Default reset type is L3 reset */
	if (!level) {
		/*
		 * Encoding: U-Boot reset command expects decimal argument,
		 * Boot strap val: Bits[3:0]
		 * reset level: Bits[7:4]
		 */
		strap_val = L3_RESET % 10;
		level = L3_RESET / 10;
		reset_level = level % 10;
		psci_system_reset2(reset_level, strap_val);
	} else {
		/* U-Boot cmd "reset" with any arg will trigger L1 reset */
		psci_system_reset();
	}
}
