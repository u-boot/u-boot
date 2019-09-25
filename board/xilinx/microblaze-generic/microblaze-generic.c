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
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
static struct udevice *watchdog_dev;
#endif /* !CONFIG_SPL_BUILD && CONFIG_WDT */

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
	ulong max_size, lowmem_size;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SYSRESET_MICROBLAZE)
	int ret;

	ret = device_bind_driver(gd->dm_root, "mb_soft_reset",
				 "reset_soft", NULL);
	if (ret)
		printf("Warning: No reset driver: ret=%d\n", ret);
#endif

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	max_size = gd->start_addr_sp - CONFIG_STACK_SIZE;
	max_size = round_down(max_size, SZ_16M);

	/* Linux default LOWMEM_SIZE is 0x30000000 = 768MB */
	lowmem_size = gd->ram_base + 768 * 1024 * 1024;

	env_set_addr("initrd_high", (void *)min_t(ulong, max_size,
						  lowmem_size));
	env_set_addr("fdt_high", (void *)min_t(ulong, max_size, lowmem_size));

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

	return 0;
}
