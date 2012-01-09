/*
 * board.c
 *
 * Common board functions for AM33XX based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;
struct gptimer *timer_base = (struct gptimer *)CONFIG_SYS_TIMERBASE;

/*
 * early system init of muxing and clocks.
 */
void s_init(u32 in_ddr)
{
	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	writel(0xAAAA, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;

	/* Setup the PLLs and the clocks for the peripherals */
#ifdef CONFIG_SETUP_PLL
	pll_init();
#endif
	if (!in_ddr)
		config_ddr();
}

/* Initialize timer */
void init_timer(void)
{
	/* Reset the Timer */
	writel(0x2, (&timer_base->tscir));

	/* Wait until the reset is done */
	while (readl(&timer_base->tiocp_cfg) & 1)
		;

	/* Start the Timer */
	writel(0x1, (&timer_base->tclr));
}

#if defined(CONFIG_OMAP_HSMMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0);
}
#endif
