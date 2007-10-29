/*
 * Copyright (C) 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_ARCH_CLK_H__
#define __ASM_AVR32_ARCH_CLK_H__

#include <asm/arch/chip-features.h>

#ifdef CONFIG_PLL
#define MAIN_CLK_RATE ((CFG_OSC0_HZ / CFG_PLL0_DIV) * CFG_PLL0_MUL)
#else
#define MAIN_CLK_RATE (CFG_OSC0_HZ)
#endif

static inline unsigned long get_cpu_clk_rate(void)
{
	return MAIN_CLK_RATE >> CFG_CLKDIV_CPU;
}
static inline unsigned long get_hsb_clk_rate(void)
{
	return MAIN_CLK_RATE >> CFG_CLKDIV_HSB;
}
static inline unsigned long get_pba_clk_rate(void)
{
	return MAIN_CLK_RATE >> CFG_CLKDIV_PBA;
}
static inline unsigned long get_pbb_clk_rate(void)
{
	return MAIN_CLK_RATE >> CFG_CLKDIV_PBB;
}

/* Accessors for specific devices. More will be added as needed. */
static inline unsigned long get_sdram_clk_rate(void)
{
	return get_hsb_clk_rate();
}
#ifdef AT32AP700x_CHIP_HAS_USART
static inline unsigned long get_usart_clk_rate(unsigned int dev_id)
{
	return get_pba_clk_rate();
}
#endif
#ifdef AT32AP700x_CHIP_HAS_USART
static inline unsigned long get_macb_pclk_rate(unsigned int dev_id)
{
	return get_pbb_clk_rate();
}
static inline unsigned long get_macb_hclk_rate(unsigned int dev_id)
{
	return get_hsb_clk_rate();
}
#endif
#ifdef AT32AP700x_CHIP_HAS_MMCI
static inline unsigned long get_mci_clk_rate(void)
{
	return get_pbb_clk_rate();
}
#endif

#endif /* __ASM_AVR32_ARCH_CLK_H__ */
