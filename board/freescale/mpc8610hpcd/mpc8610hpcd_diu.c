/*
 * Copyright 2007-2011 Freescale Semiconductor, Inc.
 * Authors: York Sun <yorksun@freescale.com>
 *          Timur Tabi <timur@freescale.com>
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
#include "../common/pixis.h"

#define PX_BRDCFG0_DLINK	0x10
#define PX_BRDCFG0_DVISEL	0x08

void diu_set_pixel_clock(unsigned int pixclock)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	volatile unsigned int *guts_clkdvdr = &gur->clkdvdr;
	unsigned long speed_ccb, temp, pixval;

	speed_ccb = get_bus_freq(0);
	temp = 1000000000/pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	debug("DIU pixval = %lu\n", pixval);

	/* Modify PXCLK in GUTS CLKDVDR */
	debug("DIU: Current value of CLKDVDR = 0x%08x\n", *guts_clkdvdr);
	temp = *guts_clkdvdr & 0x2000FFFF;
	*guts_clkdvdr = temp;				/* turn off clock */
	*guts_clkdvdr = temp | 0x80000000 | ((pixval & 0x1F) << 16);
	debug("DIU: Modified value of CLKDVDR = 0x%08x\n", *guts_clkdvdr);
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	const char *name;
	int gamma_fix = 0;
	u32 pixel_format = 0x88883316;
	u8 temp;

	temp = in_8(&pixis->brdcfg0);

	if (strncmp(port, "dlvds", 5) == 0) {
		/* Dual link LVDS */
		gamma_fix = 1;
		temp &= ~(PX_BRDCFG0_DLINK | PX_BRDCFG0_DVISEL);
		name = "Dual-Link LVDS";
	} else if (strncmp(port, "lvds", 4) == 0) {
		/* Single link LVDS */
		temp = (temp & ~PX_BRDCFG0_DVISEL) | PX_BRDCFG0_DLINK;
		name = "Single-Link LVDS";
	} else {
		/* DVI */
		if (in_8(&pixis->ver) == 1)	/* Board version */
			pixel_format = 0x88882317;
		temp |= PX_BRDCFG0_DVISEL;
		name = "DVI";
	}

	printf("DIU:   Switching to %s monitor @ %ux%u\n", name, xres, yres);
	out_8(&pixis->brdcfg0, temp);

	return fsl_diu_init(xres, yres, pixel_format, gamma_fix);
}
