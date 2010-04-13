/*
 * clocks.c - figure out sclk/cclk/vco and such
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>

/* Get the voltage input multiplier */
static u_long cached_vco_pll_ctl, cached_vco;
u_long get_vco(void)
{
	u_long msel;

	u_long pll_ctl = bfin_read_PLL_CTL();
	if (pll_ctl == cached_vco_pll_ctl)
		return cached_vco;
	else
		cached_vco_pll_ctl = pll_ctl;

	msel = (pll_ctl >> 9) & 0x3F;
	if (0 == msel)
		msel = 64;

	cached_vco = CONFIG_CLKIN_HZ;
	cached_vco >>= (1 & pll_ctl);	/* DF bit */
	cached_vco *= msel;
	return cached_vco;
}

/* Get the Core clock */
static u_long cached_cclk_pll_div, cached_cclk;
u_long get_cclk(void)
{
	u_long csel, ssel;

	if (bfin_read_PLL_STAT() & 0x1)
		return CONFIG_CLKIN_HZ;

	ssel = bfin_read_PLL_DIV();
	if (ssel == cached_cclk_pll_div)
		return cached_cclk;
	else
		cached_cclk_pll_div = ssel;

	csel = ((ssel >> 4) & 0x03);
	ssel &= 0xf;
	if (ssel && ssel < (1 << csel))	/* SCLK > CCLK */
		cached_cclk = get_vco() / ssel;
	else
		cached_cclk = get_vco() >> csel;
	return cached_cclk;
}

/* Get the System clock */
static u_long cached_sclk_pll_div, cached_sclk;
u_long get_sclk(void)
{
	u_long ssel;

	if (bfin_read_PLL_STAT() & 0x1)
		return CONFIG_CLKIN_HZ;

	ssel = bfin_read_PLL_DIV();
	if (ssel == cached_sclk_pll_div)
		return cached_sclk;
	else
		cached_sclk_pll_div = ssel;

	ssel &= 0xf;

	cached_sclk = get_vco() / ssel;
	return cached_sclk;
}
