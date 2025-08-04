// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <hang.h>
#include <spl.h>
#include <asm/arch/base_addr_soc64.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <wdt.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

u32 reset_flag(void)
{
	/* Check rstmgr.stat for warm reset status */
	u32 status = readl(SOCFPGA_RSTMGR_ADDRESS);

	/* Check whether any L4 watchdogs or SDM had triggered warm reset */
	u32 warm_reset_mask = RSTMGR_L4WD_MPU_WARMRESET_MASK;

	if (status & warm_reset_mask)
		return 0;

	return 1;
}

void board_init_f(ulong dummy)
{
	int ret;
	struct udevice *dev;

	/* Enable Async */
	asm volatile("msr daifclr, #4");

#ifdef CONFIG_SPL_BUILD
	spl_save_restore_data();
#endif

	ret = spl_early_init();
	if (ret)
		hang();

	socfpga_get_sys_mgr_addr();
	socfpga_get_managers_addr();

	sysmgr_pinmux_init();

	/* Ensure watchdog is paused when debugging is happening */
	writel(SYSMGR_WDDBG_PAUSE_ALL_CPU,
	       socfpga_get_sysmgr_addr() + SYSMGR_SOC64_WDDBG);

	timer_init();

	mbox_init();

	mbox_hps_stage_notify(HPS_EXECUTION_STATE_FSBL);

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

	ret = uclass_get_device_by_name(UCLASS_NOP, "socfpga-ccu-config", &dev);
	if (ret) {
		printf("HPS CCU settings init failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_NOP, "socfpga-firewall-config", &dev);
	if (ret) {
		printf("HPS firewall settings init failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_POWER_DOMAIN, 0, &dev);
	if (ret) {
		debug("PSS SRAM power-off failed: %d\n", ret);
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

	/* Enable non secure access to ocram */
	clrbits_le32(SOCFPGA_OCRAM_FIREWALL_ADDRESS + 0x18, BIT(0));
}
