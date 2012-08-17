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
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/common_def.h>
#include <asm/io.h>
#include <asm/omap_common.h>

DECLARE_GLOBAL_DATA_PTR;

struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;
struct gptimer *timer_base = (struct gptimer *)CONFIG_SYS_TIMERBASE;
struct uart_sys *uart_base = (struct uart_sys *)DEFAULT_UART_BASE;

/* UART Defines */
#ifdef CONFIG_SPL_BUILD
#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)
#endif

/*
 * early system init of muxing and clocks.
 */
void s_init(void)
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

#ifdef CONFIG_SPL_BUILD
	/* Setup the PLLs and the clocks for the peripherals */
	pll_init();

	/* UART softreset */
	u32 regVal;

	enable_uart0_pin_mux();

	regVal = readl(&uart_base->uartsyscfg);
	regVal |= UART_RESET;
	writel(regVal, &uart_base->uartsyscfg);
	while ((readl(&uart_base->uartsyssts) &
		UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK)
		;

	/* Disable smart idle */
	regVal = readl(&uart_base->uartsyscfg);
	regVal |= UART_SMART_IDLE_EN;
	writel(regVal, &uart_base->uartsyscfg);

	/* Initialize the Timer */
	init_timer();

	preloader_console_init();

	config_ddr();
#endif

	/* Enable MMC0 */
	enable_mmc0_pin_mux();
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

void setup_clocks_for_console(void)
{
	/* Not yet implemented */
	return;
}
