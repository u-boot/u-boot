/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * Foundation,
 */

/*
 * File:		speed.c
 *
 * Discription:		Provides cpu speed calculation
 *
 */

#include <common.h>
#include <mpc5xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Get cpu and bus clock
 */
int get_clocks (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

#ifndef	CONFIG_5xx_GCLK_FREQ
	uint divf = (immr->im_clkrst.car_plprcr & PLPRCR_DIVF_MSK);
	uint mf = ((immr->im_clkrst.car_plprcr & PLPRCR_MF_MSK) >> PLPRCR_MF_SHIFT);
	ulong vcoout;

	vcoout = (CONFIG_SYS_OSC_CLK / (divf + 1)) * (mf + 1) * 2;
	if(immr->im_clkrst.car_plprcr & PLPRCR_CSRC_MSK) {
		gd->cpu_clk = vcoout / (2^(((immr->im_clkrst.car_sccr & SCCR_DFNL_MSK) >> SCCR_DFNL_SHIFT) + 1));
	} else {
		gd->cpu_clk = vcoout / (2^(immr->im_clkrst.car_sccr & SCCR_DFNH_MSK));
	}

#else /* CONFIG_5xx_GCLK_FREQ */
	gd->bus_clk = CONFIG_5xx_GCLK_FREQ;
#endif /* CONFIG_5xx_GCLK_FREQ */

	if ((immr->im_clkrst.car_sccr & SCCR_EBDF11) == 0) {
		/* No Bus Divider active */
		gd->bus_clk = gd->cpu_clk;
	} else {
		/* CLKOUT is GCLK / 2 */
		gd->bus_clk = gd->cpu_clk / 2;
	}
	return (0);
}
