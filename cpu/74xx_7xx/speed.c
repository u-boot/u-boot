/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <74xx_7xx.h>
#include <asm/processor.h>

#ifdef CONFIG_AMIGAONEG3SE
#include "../board/MAI/AmigaOneG3SE/via686.h"
#endif

static const int hid1_multipliers_x_10[] = {
	25,	/* 0000 - 2.5x */
	75,	/* 0001 - 7.5x */
	70,	/* 0010 - 7x */
	10,	/* 0011 - bypass */
	20,	/* 0100 - 2x */
	65,	/* 0101 - 6.5x */
	100,	/* 0110 - 10x */
	45,	/* 0111 - 4.5x */
	30,	/* 1000 - 3x */
	55,	/* 1001 - 5.5x */
	40,	/* 1010 - 4x */
	50,	/* 1011 - 5x */
	80,	/* 1100 - 8x */
	60,	/* 1101 - 6x */
	35,	/* 1110 - 3.5x */
	0	/* 1111 - off */
};

/* ------------------------------------------------------------------------- */

/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2)
 *
 * (Approx. GCLK frequency in Hz)
 */

int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong clock = CFG_BUS_CLK * \
		      hid1_multipliers_x_10[get_hid1 () >> 28] / 10;
	gd->cpu_clk = clock;
	gd->bus_clk = CFG_BUS_CLK;

	return (0);
}

/* ------------------------------------------------------------------------- */

#if 0	/* disabled XXX - use global data instead */
ulong get_bus_freq (ulong gclk_freq)
{
	return CFG_BUS_CLK;
}
#endif /* 0 */

/* ------------------------------------------------------------------------- */
