/*
 * Copyright 2015 - 2016 Xilinx, Inc.
 *
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

void board_init_f(ulong dummy)
{
	psu_init();
	board_early_init_r();

#ifdef CONFIG_DEBUG_UART
	/* Uart debug for sure */
	debug_uart_init();
	puts("Debug uart enabled\n"); /* or printch() */
#endif
	/* Delay is required for clocks to be propagated */
	udelay(1000000);

	/* Clear the BSS */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* No need to call timer init - it is empty for ZynqMP */
	board_init_r(NULL, 0);
}

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	preloader_console_init();
	board_init();
}
#endif

u32 spl_boot_device(void)
{
	u32 reg = 0;
	u8 bootmode;

	reg = readl(&crlapb_base->boot_mode);
	bootmode = reg & BOOT_MODES_MASK;

	switch (bootmode) {
	case JTAG_MODE:
		return BOOT_DEVICE_RAM;
#ifdef CONFIG_SPL_MMC_SUPPORT
	case EMMC_MODE:
	case SD_MODE:
	case SD_MODE1:
		return BOOT_DEVICE_MMC1;
#endif
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	return 0;
}

u32 spl_boot_mode(const u32 boot_device)
{
	switch (spl_boot_device()) {
	case BOOT_DEVICE_RAM:
		return 0;
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_FS;
	default:
		puts("spl: error: unsupported device\n");
		hang();
	}
}

__weak void psu_init(void)
{
	 /*
	  * This function is overridden by the one in
	  * board/xilinx/zynqmp/(platform)/psu_init_gpl.c, if it exists.
	  */
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
