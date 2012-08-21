/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#include <common.h>

#ifdef CONFIG_MX31_HCLK_FREQ
#define MXC_HCLK	CONFIG_MX31_HCLK_FREQ
#else
#define MXC_HCLK	26000000
#endif

#ifdef CONFIG_MX31_CLK32
#define MXC_CLK32	CONFIG_MX31_CLK32
#else
#define MXC_CLK32	32768
#endif

enum mxc_clock {
	MXC_ARM_CLK,
	MXC_IPG_CLK,
	MXC_IPG_PERCLK,
	MXC_CSPI_CLK,
	MXC_UART_CLK,
	MXC_IPU_CLK,
	MXC_ESDHC_CLK,
};

unsigned int mxc_get_clock(enum mxc_clock clk);
extern u32 imx_get_uartclk(void);
extern void mx31_gpio_mux(unsigned long mode);
extern void mx31_set_pad(enum iomux_pins pin, u32 config);
extern void mx31_set_gpr(enum iomux_gp_func gp, char en);

void mx31_uart1_hw_init(void);
void mx31_uart2_hw_init(void);
void mx31_spi2_hw_init(void);
void mxc_hw_watchdog_enable(void);
void mxc_hw_watchdog_reset(void);

#endif /* __ASM_ARCH_CLOCK_H */
