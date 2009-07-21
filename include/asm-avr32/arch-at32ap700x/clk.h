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
#include <asm/arch/portmux.h>

#ifdef CONFIG_PLL
#define PLL0_RATE	((CONFIG_SYS_OSC0_HZ / CONFIG_SYS_PLL0_DIV)	\
				* CONFIG_SYS_PLL0_MUL)
#define MAIN_CLK_RATE	PLL0_RATE
#else
#define MAIN_CLK_RATE	(CONFIG_SYS_OSC0_HZ)
#endif

static inline unsigned long get_cpu_clk_rate(void)
{
	return MAIN_CLK_RATE >> CONFIG_SYS_CLKDIV_CPU;
}
static inline unsigned long get_hsb_clk_rate(void)
{
	return MAIN_CLK_RATE >> CONFIG_SYS_CLKDIV_HSB;
}
static inline unsigned long get_pba_clk_rate(void)
{
	return MAIN_CLK_RATE >> CONFIG_SYS_CLKDIV_PBA;
}
static inline unsigned long get_pbb_clk_rate(void)
{
	return MAIN_CLK_RATE >> CONFIG_SYS_CLKDIV_PBB;
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
#ifdef AT32AP700x_CHIP_HAS_MACB
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
#ifdef AT32AP700x_CHIP_HAS_SPI
static inline unsigned long get_spi_clk_rate(unsigned int dev_id)
{
	return get_pba_clk_rate();
}
#endif
#ifdef AT32AP700x_CHIP_HAS_LCDC
static inline unsigned long get_lcdc_clk_rate(unsigned int dev_id)
{
	return get_hsb_clk_rate();
}
#endif

extern void clk_init(void);

/* Board code may need the SDRAM base clock as a compile-time constant */
#define SDRAMC_BUS_HZ	(MAIN_CLK_RATE >> CONFIG_SYS_CLKDIV_HSB)

/* Generic clock control */
enum gclk_parent {
	GCLK_PARENT_OSC0 = 0,
	GCLK_PARENT_OSC1 = 1,
	GCLK_PARENT_PLL0 = 2,
	GCLK_PARENT_PLL1 = 3,
};

/* Some generic clocks have specific roles */
#define GCLK_DAC_SAMPLE_CLK	6
#define GCLK_LCDC_PIXCLK	7

extern unsigned long __gclk_set_rate(unsigned int id, enum gclk_parent parent,
		unsigned long rate, unsigned long parent_rate);

/**
 * gclk_set_rate - configure and enable a generic clock
 * @id: Which GCLK[id] to enable
 * @parent: Parent clock feeding the GCLK
 * @rate: Target rate of the GCLK in Hz
 *
 * Returns the actual GCLK rate in Hz, after rounding to the nearest
 * supported rate.
 *
 * All three parameters are usually constant, hence the inline.
 */
static inline unsigned long gclk_set_rate(unsigned int id,
		enum gclk_parent parent, unsigned long rate)
{
	unsigned long parent_rate;

	if (id > 7)
		return 0;

	switch (parent) {
	case GCLK_PARENT_OSC0:
		parent_rate = CONFIG_SYS_OSC0_HZ;
		break;
#ifdef CONFIG_SYS_OSC1_HZ
	case GCLK_PARENT_OSC1:
		parent_rate = CONFIG_SYS_OSC1_HZ;
		break;
#endif
#ifdef PLL0_RATE
	case GCLK_PARENT_PLL0:
		parent_rate = PLL0_RATE;
		break;
#endif
#ifdef PLL1_RATE
	case GCLK_PARENT_PLL1:
		parent_rate = PLL1_RATE;
		break;
#endif
	default:
		parent_rate = 0;
		break;
	}

	return __gclk_set_rate(id, parent, rate, parent_rate);
}

/**
 * gclk_enable_output - enable output on a GCLK pin
 * @id: Which GCLK[id] pin to enable
 * @drive_strength: Drive strength of external GCLK pin, if applicable
 */
static inline void gclk_enable_output(unsigned int id,
		unsigned long drive_strength)
{
	switch (id) {
	case 0:
		portmux_select_peripheral(PORTMUX_PORT_A, 1 << 30,
				PORTMUX_FUNC_A, drive_strength);
		break;
	case 1:
		portmux_select_peripheral(PORTMUX_PORT_A, 1 << 31,
				PORTMUX_FUNC_A, drive_strength);
		break;
	case 2:
		portmux_select_peripheral(PORTMUX_PORT_B, 1 << 19,
				PORTMUX_FUNC_A, drive_strength);
		break;
	case 3:
		portmux_select_peripheral(PORTMUX_PORT_B, 1 << 29,
				PORTMUX_FUNC_A, drive_strength);
		break;
	case 4:
		portmux_select_peripheral(PORTMUX_PORT_B, 1 << 30,
				PORTMUX_FUNC_A, drive_strength);
		break;
	}
}

#endif /* __ASM_AVR32_ARCH_CLK_H__ */
