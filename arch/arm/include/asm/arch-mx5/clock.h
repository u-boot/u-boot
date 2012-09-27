/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
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

#ifdef CONFIG_SYS_MX5_HCLK
#define MXC_HCLK	CONFIG_SYS_MX5_HCLK
#else
#define MXC_HCLK	24000000
#endif

#ifdef CONFIG_SYS_MX5_CLK32
#define MXC_CLK32	CONFIG_SYS_MX5_CLK32
#else
#define MXC_CLK32	32768
#endif

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_AHB_CLK,
	MXC_IPG_CLK,
	MXC_IPG_PERCLK,
	MXC_UART_CLK,
	MXC_CSPI_CLK,
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_ESDHC3_CLK,
	MXC_ESDHC4_CLK,
	MXC_FEC_CLK,
	MXC_SATA_CLK,
	MXC_DDR_CLK,
	MXC_NFC_CLK,
	MXC_PERIPH_CLK,
	MXC_I2C_CLK,
};

u32 imx_get_uartclk(void);
u32 imx_get_fecclk(void);
unsigned int mxc_get_clock(enum mxc_clock clk);
int mxc_set_clock(u32 ref, u32 freq, u32 clk_type);
void set_usb_phy_clk(void);
void enable_usb_phy1_clk(unsigned char enable);
void enable_usb_phy2_clk(unsigned char enable);
void set_usboh3_clk(void);
void enable_usboh3_clk(unsigned char enable);
void mxc_set_sata_internal_clock(void);
int enable_i2c_clk(unsigned char enable, unsigned i2c_num);

#endif /* __ASM_ARCH_CLOCK_H */
