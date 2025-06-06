// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025, Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/spl.h>
#include <cpu_func.h>
#include <dm.h>
#include <hang.h>
#include <spl.h>

u32 spl_boot_device(void)
{
	/*
	 * We don't bother to load proper U-Boot from an external device as
	 * it fits in the integrated SRAM nicely.
	 */
	return BOOT_DEVICE_RAM;
}

void board_init_f(ulong dummy)
{
	int ret = spl_early_init();
	struct udevice *dev;

	if (ret)
		panic("spl_early_init() failed %d\n", ret);

	preloader_console_init();

	/*
	 * Manually bind CPU ahead of time to make sure in-core timers are
	 * available in SPL.
	 */
	ret = uclass_get_device(UCLASS_CPU, 0, &dev);
	if (ret)
		panic("failed to bind CPU: %d\n", ret);

	riscv_cpu_setup();
	th1520_kick_secondary_cores();

	spl_dram_init();

	icache_enable();
	dcache_enable();

	th1520_invalidate_pmp();
}
