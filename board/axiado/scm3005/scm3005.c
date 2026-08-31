// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021-2026 Axiado Corporation (or its affiliates).
 */

#include <config.h>
#include <dm.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/spin_table.h>
#include <asm/system.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region axiado_ax3005_mem_map[] = {
	{ /* Peripherals including UART */
	  .virt = 0x00000000UL,
	  .phys = 0x00000000UL,
	  .size = 0x4A000000UL, /* 0 to 0x4A000000: peripherals */
	  .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		   PTE_BLOCK_PXN | PTE_BLOCK_UXN },
	{ .virt = 0x80000000UL,
	  .phys = 0x80000000UL,
	  .size = 0x80000000UL,
	  .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE },
	{
		0,
	}
};

struct mm_region *mem_map = axiado_ax3005_mem_map;

/*
 * Accept any FIT configuration name - the board loads a single FIT image
 * and the first matching config is used.
 */
int board_fit_config_name_match(const char *name)
{
	return 0;
}

/*
 * ft_board_setup - restore cpu-release-addr after relocation
 *
 * arch_fixup_fdt() / spin_table_update_dt() overwrites cpu-release-addr
 * with U-Boot's relocated address.  Restore the pre-relocation physical
 * address so secondary cores spin on the correct location.
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int cpus_offset, offset;
	const char *prop;
	int ret;
	u64 cpu_release_addr = (u64)&spin_table_cpu_release_addr - gd->reloc_off;

	cpus_offset = fdt_path_offset(blob, "/cpus");
	if (cpus_offset < 0)
		return 0;

	for (offset = fdt_first_subnode(blob, cpus_offset); offset >= 0;
	     offset = fdt_next_subnode(blob, offset)) {
		prop = fdt_getprop(blob, offset, "device_type", NULL);
		if (!prop || strcmp(prop, "cpu"))
			continue;

		prop = fdt_getprop(blob, offset, "enable-method", NULL);
		if (!prop || strcmp(prop, "spin-table"))
			continue;

		ret = fdt_setprop_u64(blob, offset, "cpu-release-addr",
				      cpu_release_addr);
		if (ret) {
			printf("WARNING: Failed to restore cpu-release-addr\n");
			return ret;
		}
	}

	return 0;
}

/*
 * dram_init - DDR is initialized by firmware, just setting size
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);
	return 0;
}

/*
 * the SOC uses single bank, non-interleaving
 */
int dram_init_banksize(void)
{
	gd->dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->dram[0].size = CFG_SYS_SDRAM_SIZE;
	return 0;
}

/*
 * timer_init - enable the AX3005 platform system timer
 *
 * CNTFRQ_EL0 is already set by arch/arm/cpu/armv8/start.S using
 * CONFIG_COUNTER_FREQUENCY from the defconfig.
 *
 * SYS_TIMER_CTRL (0x48016000) is the AX3005 system timer control
 * register — writing SYS_TIMER_ENABLE starts the counter that feeds
 * the ARM generic timer.
 */
int timer_init(void)
{
	writel(SYS_TIMER_ENABLE, SYS_TIMER_CTRL);
	return 0;
}

int board_init(void)
{
	return 0;
}

void reset_cpu(void)
{
	/* For later ARM_PSCI_FW or watchdog reset */
}
