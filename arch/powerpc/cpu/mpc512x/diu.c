/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 * York Sun <yorksun@freescale.com>
 *
 * FSL DIU Framebuffer driver
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
