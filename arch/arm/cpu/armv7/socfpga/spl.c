/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <version.h>
#include <image.h>
#include <asm/arch/reset_manager.h>
#include <spl.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/freeze_controller.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

/*
 * Board initialization after bss clearance
 */
void spl_board_init(void)
{
#ifndef CONFIG_SOCFPGA_VIRTUAL_TARGET
	debug("Freezing all I/O banks\n");
	/* freeze all IO banks */
	sys_mgr_frzctrl_freeze_req();

	/* configure the pin muxing through system manager */
	sysmgr_pinmux_init();
#endif /* CONFIG_SOCFPGA_VIRTUAL_TARGET */

	/* de-assert reset for peripherals and bridges based on handoff */
	reset_deassert_peripherals_handoff();

	debug("Unfreezing/Thaw all I/O banks\n");
	/* unfreeze / thaw all IO banks */
	sys_mgr_frzctrl_thaw_req();

	/* enable console uart printing */
	preloader_console_init();
}
