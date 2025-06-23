// SPDX-License-Identifier: GPL-2.0

#include <fdtdec.h>
#include <init.h>
#include <sysreset.h>
#include <asm/armv8/mmu.h>
#include <asm/system.h>

int print_cpuinfo(void)
{
	printf("CPU:   Airoha AN7581\n");
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

void reset_cpu(void)
{
	psci_system_reset();
}

static struct mm_region an7581_mem_map[] = {
	{
		/* DDR */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};
struct mm_region *mem_map = an7581_mem_map;
