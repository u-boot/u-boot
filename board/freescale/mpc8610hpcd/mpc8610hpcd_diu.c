/*
 * Copyright 2007 Freescale Semiconductor, Inc.
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

int platform_diu_init(unsigned int *xres, unsigned int *yres)
{
	char *monitor_port;
	int gamma_fix;
	unsigned int pixel_format;
	unsigned char tmp_val;
	unsigned char pixis_arch;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	tmp_val = in_8(pixis_base + PIXIS_BRDCFG0);
	pixis_arch = in_8(pixis_base + PIXIS_VER);

	monitor_port = getenv("monitor");
	if (!strncmp(monitor_port, "0", 1)) {	/* 0 - DVI */
		*xres = 1280;
		*yres = 1024;
		if (pixis_arch == 0x01)
			pixel_format = 0x88882317;
		else
			pixel_format = 0x88883316;
		gamma_fix = 0;
		out_8(pixis_base + PIXIS_BRDCFG0, tmp_val | 0x08);

	} else if (!strncmp(monitor_port, "1", 1)) { /* 1 - Single link LVDS */
		*xres = 1024;
		*yres = 768;
		pixel_format = 0x88883316;
		gamma_fix = 0;
		out_8(pixis_base + PIXIS_BRDCFG0, (tmp_val & 0xf7) | 0x10);

	} else if (!strncmp(monitor_port, "2", 1)) { /* 2 - Double link LVDS */
		*xres = 1280;
		*yres = 1024;
		pixel_format = 0x88883316;
		gamma_fix = 1;
		out_8(pixis_base + PIXIS_BRDCFG0, tmp_val & 0xe7);

	} else {	/* DVI */
		*xres = 1280;
		*yres = 1024;
		pixel_format = 0x88882317;
		gamma_fix = 0;
		out_8(pixis_base + PIXIS_BRDCFG0, tmp_val | 0x08);
	}

	return fsl_diu_init(*xres, pixel_format, gamma_fix);
}
