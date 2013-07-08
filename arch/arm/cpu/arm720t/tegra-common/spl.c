/*
 * (C) Copyright 2012
 * NVIDIA Inc, <www.nvidia.com>
 *
 * Allen Martin <amartin@nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch/spl.h>
#include "cpu.h"

void spl_board_init(void)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;

	/* enable JTAG */
	writel(0xC0, &pmt->pmt_cfg_ctl);

	board_init_uart_f();

	/* Initialize periph GPIOs */
	gpio_early_init_uart();

	clock_early_init();
	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	debug("image entry point: 0x%X\n", spl_image->entry_point);

	start_cpu((u32)spl_image->entry_point);
	halt_avp();
}
