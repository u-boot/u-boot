/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 * York Sun <yorksun@freescale.com>
 *
 * FSL DIU Framebuffer driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#include <fsl_diu_fb.h>

DECLARE_GLOBAL_DATA_PTR;

void diu_set_pixel_clock(unsigned int pixclock)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile clk512x_t *clk = &immap->clk;
	volatile unsigned int *clkdvdr = &clk->scfr[0];
	unsigned long speed_ccb, temp, pixval;

	speed_ccb = get_bus_freq(0) * 4;
	temp = 1000000000/pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	debug("DIU pixval = %lu\n", pixval);

	/* Modify PXCLK in GUTS CLKDVDR */
	debug("DIU: Current value of CLKDVDR = 0x%08x\n", in_be32(clkdvdr));
	temp = in_be32(clkdvdr) & 0xFFFFFF00;
	out_be32(clkdvdr, temp | (pixval & 0xFF));
	debug("DIU: Modified value of CLKDVDR = 0x%08x\n", in_be32(clkdvdr));
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	unsigned int pixel_format = 0x88883316;

	debug("mpc5121_diu_init\n");
	return fsl_diu_init(xres, yres, pixel_format, 0);
}
