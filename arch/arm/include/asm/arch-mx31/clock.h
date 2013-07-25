/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
	MXC_I2C_CLK,
};

unsigned int mxc_get_clock(enum mxc_clock clk);
extern u32 imx_get_uartclk(void);
extern void mx31_gpio_mux(unsigned long mode);
extern void mx31_set_pad(enum iomux_pins pin, u32 config);
extern void mx31_set_gpr(enum iomux_gp_func gp, char en);

void mx31_uart1_hw_init(void);
void mx31_uart2_hw_init(void);
void mx31_spi2_hw_init(void);

#endif /* __ASM_ARCH_CLOCK_H */
