// SPDX-License-Identifier: GPL-2.0
/*
 * R-Car Gen3 recovery SPL
 *
 * Copyright (C) 2019 Marek Vasut <marek.vasut@gmail.com>
 */

#include <init.h>
#include <asm/io.h>
#include <spl.h>

#define RCAR_CNTC_BASE	0xE6080000
#define CNTCR_EN	BIT(0)

void board_init_f(ulong dummy)
{
	writel(CNTCR_EN, RCAR_CNTC_BASE);
	timer_init();
}

void spl_board_init(void)
{
	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_UART;
}

void s_init(void)
{
}

void reset_cpu(void)
{
}
