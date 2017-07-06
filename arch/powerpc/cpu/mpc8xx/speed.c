/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

void get_brgclk(uint sccr)
{
	uint divider = 0;

	switch ((sccr & SCCR_DFBRG11) >> 11) {
	case 0:
		divider = 1;
		break;
	case 1:
		divider = 4;
		break;
	case 2:
		divider = 16;
		break;
	case 3:
		divider = 64;
		break;
	}
	gd->arch.brg_clk = gd->cpu_clk / divider;
}

/*
 * get_clocks() fills in gd->cpu_clock depending on CONFIG_8xx_GCLK_FREQ
 */
int get_clocks(void)
{
	uint immr = get_immr(0);	/* Return full IMMR contents */
	immap_t __iomem *immap = (immap_t __iomem *)(immr & 0xFFFF0000);
	uint sccr = in_be32(&immap->im_clkrst.car_sccr);
	/*
	 * If for some reason measuring the gclk frequency won't
	 * work, we return the hardwired value.
	 * (For example, the cogent CMA286-60 CPU module has no
	 * separate oscillator for PITRTCLK)
	 */
	gd->cpu_clk = CONFIG_8xx_GCLK_FREQ;

	if ((sccr & SCCR_EBDF11) == 0) {
		/* No Bus Divider active */
		gd->bus_clk = gd->cpu_clk;
	} else {
		/* The MPC8xx has only one BDF: half clock speed */
		gd->bus_clk = gd->cpu_clk / 2;
	}

	get_brgclk(sccr);

	return 0;
}
