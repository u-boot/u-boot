// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include "sdram_soc32.h"
#include <string.h>
#include <hang.h>
#include <linux/sizes.h>
#include <cpu_func.h>
#include <watchdog.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <asm/system.h>
#if !IS_ENABLED(CONFIG_WATCHDOG) && !CONFIG_IS_ENABLED(WDT)
#include <asm/arch/reset_manager.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define PGTABLE_OFF	0x4000
#define PGTABLE_RESERVE (PGTABLE_OFF + PGTABLE_SIZE)

#if IS_ENABLED(CONFIG_SOCFPGA_ECC_SUPPORT)
static void socfpga_prepare_watchdog_for_long_op(void)
{
#if !IS_ENABLED(CONFIG_WATCHDOG) && !CONFIG_IS_ENABLED(WDT)
	/*
	 * No watchdog driver is active in U-Boot. The bootrom or a
	 * previous boot stage may have left L4WD0 running. Assert and
	 * deassert its reset to stop it before the long ECC scrub,
	 * preventing a spurious watchdog reset during DDR init.
	 */
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
#endif
}

/* Initialize SDRAM ECC bits to avoid false DBE */
void sdram_init_ecc_bits(void)
{
	u32 start;
	phys_addr_t start_addr;
	phys_size_t size, size_init;

	start = get_timer(0);

	start_addr = gd->bd->bi_dram[0].start;
	size = gd->bd->bi_dram[0].size;

	printf("DDRCAL: Scrubbing ECC RAM (%lu MiB).\n",
	       (ulong)(size >> 20));

	if (size <= PGTABLE_RESERVE) {
		printf("DDRCAL: Error: DRAM size %#llx smaller than scrub reserve %#x\n",
		       (unsigned long long)size, PGTABLE_RESERVE);
		hang();
	}

	memset((void *)start_addr, 0, PGTABLE_RESERVE);
	gd->arch.tlb_addr = start_addr + PGTABLE_OFF;
	gd->arch.tlb_size = PGTABLE_SIZE;
	start_addr += PGTABLE_RESERVE;
	size -= PGTABLE_RESERVE;

	dcache_enable();

	socfpga_prepare_watchdog_for_long_op();

	while (size > 0) {
		size_init = min_t(phys_size_t, (phys_size_t)SZ_1G, size);
		memset((void *)start_addr, 0, size_init);
		size -= size_init;
		start_addr += size_init;

		/* Service DM watchdog cyclic callbacks */
		schedule();
	}

	dcache_disable();

	printf("DDRCAL: SDRAM-ECC initialized successfully with %u ms\n",
	       (u32)get_timer(start));
}
#endif
