/*
 * Copyright (C) 2012 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <asm/blackfin.h>
#ifdef PLL_CTL
#include <asm/mach-common/bits/pll.h>
# define pll_is_bypassed() (bfin_read_PLL_CTL() & BYPASS)
#else
#include <asm/mach-common/bits/cgu.h>
# define pll_is_bypassed() (bfin_read_CGU_STAT() & PLLBP)
# define bfin_read_PLL_CTL() bfin_read_CGU_CTL()
# define bfin_read_PLL_DIV() bfin_read_CGU_DIV()
# define SSEL SYSSEL
# define SSEL_P SYSSEL_P
#endif

__attribute__((always_inline))
static inline uint32_t early_division(uint32_t dividend, uint32_t divisor)
{
	uint32_t quotient;
	uint32_t i, j;

	for (quotient = 1, i = 1; dividend > divisor; ++i) {
		j = divisor << i;
		if (j > dividend || (j & 0x80000000)) {
			--i;
			quotient += (1 << i);
			dividend -= (divisor << i);
			i = 0;
		}
	}

	return quotient;
}

__attribute__((always_inline))
static inline uint32_t early_get_uart_clk(void)
{
	uint32_t msel, pll_ctl, vco;
	uint32_t div, ssel, sclk, uclk;

	pll_ctl = bfin_read_PLL_CTL();
	msel = (pll_ctl & MSEL) >> MSEL_P;
	if (msel == 0)
		msel = (MSEL >> MSEL_P) + 1;

	vco = (CONFIG_CLKIN_HZ >> (pll_ctl & DF)) * msel;
	sclk = vco;
	if (!pll_is_bypassed()) {
		div = bfin_read_PLL_DIV();
		ssel = (div & SSEL) >> SSEL_P;
#if CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS
		sclk = vco/ssel;
#else
		sclk = early_division(vco, ssel);
#endif
	}
	uclk = sclk;
#ifdef CGU_DIV
	ssel = (div & S0SEL) >> S0SEL_P;
	uclk = early_division(sclk, ssel);
#endif
	return uclk;
}

extern u_long get_vco(void);
extern u_long get_cclk(void);
extern u_long get_sclk(void);

#ifdef CGU_DIV
extern u_long get_sclk0(void);
extern u_long get_sclk1(void);
extern u_long get_dclk(void);
# define get_uart_clk get_sclk0
# define get_i2c_clk get_sclk0
# define get_spi_clk get_sclk1
#else
# define get_uart_clk get_sclk
# define get_i2c_clk get_sclk
# define get_spi_clk get_sclk
#endif

#endif
