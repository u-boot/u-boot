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

#ifndef __ASM_ARCH_MX31_H
#define __ASM_ARCH_MX31_H

extern u32 mx31_get_ipg_clk(void);
#define imx_get_uartclk mx31_get_ipg_clk
extern void mx31_gpio_mux(unsigned long mode);
extern void mx31_set_pad(enum iomux_pins pin, u32 config);

void mx31_uart1_hw_init(void);
void mx31_spi2_hw_init(void);

#endif /* __ASM_ARCH_MX31_H */
