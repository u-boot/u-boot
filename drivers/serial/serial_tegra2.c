/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include "serial_tegra2.h"

static void setup_uart(struct uart_ctlr *u)
{
	u32 reg;

	/* Prepare the divisor value */
	reg = NVRM_PLLP_FIXED_FREQ_KHZ * 1000 / NV_DEFAULT_DEBUG_BAUD / 16;

	/* Set up UART parameters */
	writel(UART_LCR_DLAB, &u->uart_lcr);
	writel(reg, &u->uart_thr_dlab_0);
	writel(0, &u->uart_ier_dlab_0);
	writel(0, &u->uart_lcr);			/* clear DLAB */
	writel((UART_FCR_TRIGGER_3 | UART_FCR_FIFO_EN | \
		UART_FCR_CLEAR_XMIT | UART_FCR_CLEAR_RCVR), &u->uart_iir_fcr);
	writel(0, &u->uart_ier_dlab_0);
	writel(UART_LCR_WLS_8, &u->uart_lcr);	/* 8N1 */
	writel(UART_MCR_RTS, &u->uart_mcr);
	writel(0, &u->uart_msr);
	writel(0, &u->uart_spr);
	writel(0, &u->uart_irda_csr);
	writel(0, &u->uart_asr);
	writel((UART_FCR_TRIGGER_3 | UART_FCR_FIFO_EN), &u->uart_iir_fcr);

	/* Flush any old characters out of the RX FIFO */
	reg = readl(&u->uart_lsr);

	while (reg & UART_LSR_DR) {
		reg = readl(&u->uart_thr_dlab_0);
		reg = readl(&u->uart_lsr);
	}
}

/*
 * Routine: uart_init
 * Description: init the UART clocks, muxes, and baudrate/parity/etc.
 */
void uart_init(void)
{
	struct uart_ctlr *uart = (struct uart_ctlr *)NV_PA_APB_UARTD_BASE;
#if defined(CONFIG_TEGRA2_ENABLE_UARTD)
	setup_uart(uart);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTD */
#if defined(CONFIG_TEGRA2_ENABLE_UARTA)
	uart = (struct uart_ctlr *)NV_PA_APB_UARTA_BASE;

	setup_uart(uart);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTA */
}
