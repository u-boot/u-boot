/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Authors: Timur Tabi <timur@freescale.com>
 *
 * FSL DIU Framebuffer driver
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <video_fb.h>
#include "../common/ngpixis.h"
#include <fsl_diu_fb.h>

#define PX_BRDCFG0_ELBC_DIU	0x02

#define PX_BRDCFG1_DVIEN	0x80
#define PX_BRDCFG1_DFPEN	0x40
#define PX_BRDCFG1_BACKLIGHT	0x20

/*
 * DIU Area Descriptor
 *
 * Note that we need to byte-swap the value before it's written to the AD
 * register.  So even though the registers don't look like they're in the same
 * bit positions as they are on the MPC8610, the same value is written to the
 * AD register on the MPC8610 and on the P1022.
 */
#define AD_BYTE_F		0x10000000
#define AD_ALPHA_C_SHIFT	25
#define AD_BLUE_C_SHIFT		23
#define AD_GREEN_C_SHIFT	21
#define AD_RED_C_SHIFT		19
#define AD_PIXEL_S_SHIFT	16
#define AD_COMP_3_SHIFT		12
#define AD_COMP_2_SHIFT		8
#define AD_COMP_1_SHIFT		4
#define AD_COMP_0_SHIFT		0

void diu_set_pixel_clock(unsigned int pixclock)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned long speed_ccb, temp;
	u32 pixval;

	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	debug("DIU pixval = %lu\n", pixval);

	/* Modify PXCLK in GUTS CLKDVDR */
	temp = in_be32(&gur->clkdvdr) & 0x2000FFFF;
	out_be32(&gur->clkdvdr, temp);			/* turn off clock */
	out_be32(&gur->clkdvdr, temp | 0x80000000 | ((pixval & 0x1F) << 16));
}

int platform_diu_init(unsigned int *xres, unsigned int *yres)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	char *monitor_port;
	u32 pixel_format;
	u8 temp;

	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	temp = in_8(&pixis->brdcfg1);

	monitor_port = getenv("monitor");
	if (!strncmp(monitor_port, "1", 1)) { /* 1 - Single link LVDS */
		*xres = 1024;
		*yres = 768;
		/* Enable the DFP port, disable the DVI and the backlight */
		temp &= ~(PX_BRDCFG1_DVIEN | PX_BRDCFG1_BACKLIGHT);
		temp |= PX_BRDCFG1_DFPEN;
	} else {	/* DVI */
		*xres = 1280;
		*yres = 1024;
		/* Enable the DVI port, disable the DFP and the backlight */
		temp &= ~(PX_BRDCFG1_DFPEN | PX_BRDCFG1_BACKLIGHT);
		temp |= PX_BRDCFG1_DVIEN;
	}

	out_8(&pixis->brdcfg1, temp);

	/*
	 * Route the LAD pins to the DIU.  This will disable access to the eLBC,
	 * which means we won't be able to read/write any NOR flash addresses!
	 */
	out_8(&pixis->brdcfg0, in_8(&pixis->brdcfg0) | PX_BRDCFG0_ELBC_DIU);
	/* we must do the dummy read from eLBC to sync the write as above */
	in_8(&pixis->brdcfg0);

	/* Setting PMUXCR to switch to DVI from ELBC */
	/* Set pmuxcr to allow both i2c1 and i2c2 */
	clrsetbits_be32(&gur->pmuxcr, 0xc0000000, 0x40000000);
	in_be32(&gur->pmuxcr);

	return fsl_diu_init(*xres, pixel_format, 0);
}
