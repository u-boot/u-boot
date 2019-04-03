// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2018 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

/*
 * This is a board specific file.  It's OK to include board specific
 * header files
 */

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/lists.h>
#include <fdtdec.h>
#include <asm/processor.h>
#include <asm/microblaze_intc.h>
#include <asm/asm.h>
#include <asm/gpio.h>
#include <dm/uclass.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
static struct udevice *watchdog_dev __attribute__((section(".data"))) = NULL;
#endif /* !CONFIG_SPL_BUILD && CONFIG_WDT */

ulong ram_base;

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
};

#ifdef CONFIG_WDT
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
#if !defined(CONFIG_SPL_BUILD)
	ulong now;
	static ulong next_reset;

	if (!watchdog_dev)
		return;

	now = timer_get_us();

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		wdt_reset(watchdog_dev);
		next_reset = now + 1000;
	}
#endif /* !CONFIG_SPL_BUILD */
}
#endif /* CONFIG_WDT */

int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
	watchdog_dev = NULL;

	if (uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev)) {
		debug("Watchdog: Not found by seq!\n");
		if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
			puts("Watchdog: Not found!\n");
			return 0;
		}
	}

	wdt_start(watchdog_dev, 0, 0);
	puts("Watchdog: Started\n");
#endif /* !CONFIG_SPL_BUILD && CONFIG_WDT */
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SYSRESET_MICROBLAZE)
	int ret;

	ret = device_bind_driver(gd->dm_root, "mb_soft_reset",
				 "reset_soft", NULL);
	if (ret)
		printf("Warning: No reset driver: ret=%d\n", ret);
#endif
	return 0;
}
