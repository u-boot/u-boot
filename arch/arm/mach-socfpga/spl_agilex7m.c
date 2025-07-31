// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <wdt.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/firewall.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong dummy)
{
	int ret;
	struct udevice *dev;

	/* Enable Async */
	asm volatile("msr daifclr, #4");

#ifdef CONFIG_XPL_BUILD
	spl_save_restore_data();
#endif

	ret = spl_early_init();
	if (ret)
		hang();

	socfpga_get_sys_mgr_addr();
	socfpga_get_managers_addr();

	/* Ensure watchdog is paused when debugging is happening */
	writel(SYSMGR_WDDBG_PAUSE_ALL_CPU,
	       socfpga_get_sysmgr_addr() + SYSMGR_SOC64_WDDBG);

	/* ensure all processors are not released prior Linux boot */
	writeq(0, CPU_RELEASE_ADDR);

	timer_init();

	mbox_init();

	mbox_hps_stage_notify(HPS_EXECUTION_STATE_FSBL);

	sysmgr_pinmux_init();

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		debug("Clock init failed: %d\n", ret);
		hang();
	}

	/*
	 * Enable watchdog as early as possible before initializing other
	 * component. Watchdog need to be enabled after clock driver because
	 * it will retrieve the clock frequency from clock driver.
	 */
	if (CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	preloader_console_init();
	print_reset_info();
	cm_print_clock_quick_summary();

	ret = uclass_get_device_by_name(UCLASS_NOP, "socfpga-system-mgr-firewall", &dev);
	if (ret) {
		printf("System manager firewall configuration failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_NOP, "socfpga-l3interconnect-firewall", &dev);
	if (ret) {
		printf("L3 interconnect firewall configuration failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_CACHE, 0, &dev);
	if (ret) {
		debug("CCU init failed: %d\n", ret);
		hang();
	}

	if (IS_ENABLED(CONFIG_SPL_ALTERA_SDRAM)) {
		ret = uclass_get_device(UCLASS_RAM, 0, &dev);
		if (ret) {
			debug("DRAM init failed: %d\n", ret);
			hang();
		}
	}

	if (IS_ENABLED(CONFIG_CADENCE_QSPI))
		mbox_qspi_open();
}
