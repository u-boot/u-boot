// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include "sdram_soc32.h"
#include <string.h>
#include <linux/sizes.h>
#include <cpu_func.h>
#include <watchdog.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <asm/system.h>
#if !defined(CONFIG_HW_WATCHDOG)
#include <asm/arch/reset_manager.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define PGTABLE_OFF	0x4000

/* Initialize SDRAM ECC bits to avoid false DBE */
void sdram_init_ecc_bits(void)
{
	u32 start;
	phys_addr_t start_addr;
	phys_size_t size, size_init;

	start = get_timer(0);

	start_addr = gd->bd->bi_dram[0].start;
	size = gd->bd->bi_dram[0].size;

	printf("DDRCAL: Scrubbing ECC RAM (%ld MiB).\n", size >> 20);

	memset((void *)start_addr, 0, PGTABLE_SIZE + PGTABLE_OFF);
	gd->arch.tlb_addr = start_addr + PGTABLE_OFF;
	gd->arch.tlb_size = PGTABLE_SIZE;
	start_addr += PGTABLE_SIZE + PGTABLE_OFF;
	size -= PGTABLE_OFF + PGTABLE_SIZE;

	dcache_enable();

	while (size > 0) {
		size_init = min((phys_addr_t)SZ_1G, (phys_addr_t)size);
		memset((void *)start_addr, 0, size_init);
		size -= size_init;
		start_addr += size_init;

#ifdef CONFIG_HW_WATCHDOG
		/*
		 * In case the watchdog is enabled, make sure to
		 * (re-)configure watchdog so that the defined timeout is
		 * valid.
		 */
		debug("%s: %d\n", __func__, __LINE__);
		hw_watchdog_init();
#else
		/*
		 * If the HW watchdog is NOT enabled, make sure it is not
		 * running, because it is enabled in the preloader and
		 * causing boot loop if is not handled.
		 */
		debug("%s: %d\n", __func__, __LINE__);
		socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
		socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
#endif
	}

	dcache_disable();

	printf("DDRCAL: SDRAM-ECC initialized success with %d ms\n",
	       (u32)get_timer(start));
}
