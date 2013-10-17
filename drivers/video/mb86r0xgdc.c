/*
 * (C) Copyright 2010
 * Matthias Weisser <weisserm@arcor.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * mb86r0xgdc.c - Graphic interface for Fujitsu MB86R0x integrated graphic
 * controller.
 */

#include <common.h>

#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <video_fb.h>
#include "videomodes.h"

/*
 * 4MB (at the end of system RAM)
 */
#define VIDEO_MEM_SIZE		0x400000

#define FB_SYNC_CLK_INV		(1<<16)	/* pixel clock inverted */

/*
 * Graphic Device
 */
static GraphicDevice mb86r0x;

static void dsp_init(struct mb86r0x_gdc_dsp *dsp, char *modestr,
			u32 *videomem)
{
	struct ctfb_res_modes var_mode;
	u32 dcm1, dcm2, dcm3;
	u16 htp, hdp, hdb, hsp, vtr, vsp, vdp;
	u8 hsw, vsw;
	u32 l2m, l2em, l2oa0, l2da0, l2oa1, l2da1;
	u16 l2dx, l2dy, l2wx, l2wy, l2ww, l2wh;
	unsigned long div;
	int bpp;

	bpp = video_get_params(&var_mode, modestr);

	if (bpp == 0) {
		var_mode.xres = 640;
		var_mode.yres = 480;
		var_mode.pixclock = 39721;	/* 25MHz */
		var_mode.left_margin = 48;
		var_mode.right_margin = 16;
		var_mode.upper_margin = 33;
		var_mode.lower_margin = 10;
		var_mode.hsync_len = 96;
		var_mode.vsync_len = 2;
		var_mode.sync = 0;
		var_mode.vmode = 0;
		bpp = 15;
	}

	/* Fill memory with white */
	memset(videomem, 0xFF, var_mode.xres * var_mode.yres * 2);

	mb86r0x.winSizeX = var_mode.xres;
	mb86r0x.winSizeY = var_mode.yres;

	/* LCD base clock is ~ 660MHZ. We do calculations in kHz */
	div = 660000 / (1000000000L / var_mode.pixclock);
	if (div > 64)
		div = 64;
	if (0 == div)
		div = 1;

	dcm1 = (div - 1) << 8;
	dcm2 = 0x00000000;
	if (var_mode.sync & FB_SYNC_CLK_INV)
		dcm3 = 0x00000100;
	else
		dcm3 = 0x00000000;

	htp = var_mode.left_margin + var_mode.xres +
		var_mode.hsync_len + var_mode.right_margin;
	hdp = var_mode.xres;
	hdb = var_mode.xres;
	hsp = var_mode.xres + var_mode.right_margin;
	hsw = var_mode.hsync_len;

	vsw = var_mode.vsync_len;
	vtr = var_mode.upper_margin + var_mode.yres +
		var_mode.vsync_len + var_mode.lower_margin;
	vsp = var_mode.yres + var_mode.lower_margin;
	vdp = var_mode.yres;

	l2m =	((var_mode.yres - 1) << (0)) |
		(((var_mode.xres * 2) / 64) << (16)) |
		((1) << (31));

	l2em = (1 << 0) | (1 << 1);

	l2oa0 = mb86r0x.frameAdrs;
	l2da0 = mb86r0x.frameAdrs;
	l2oa1 = mb86r0x.frameAdrs;
	l2da1 = mb86r0x.frameAdrs;
	l2dx = 0;
	l2dy = 0;
	l2wx = 0;
	l2wy = 0;
	l2ww = var_mode.xres;
	l2wh = var_mode.yres - 1;

	writel(dcm1, &dsp->dcm1);
	writel(dcm2, &dsp->dcm2);
	writel(dcm3, &dsp->dcm3);

	writew(htp, &dsp->htp);
	writew(hdp, &dsp->hdp);
	writew(hdb, &dsp->hdb);
	writew(hsp, &dsp->hsp);
	writeb(hsw, &dsp->hsw);

	writeb(vsw, &dsp->vsw);
	writew(vtr, &dsp->vtr);
	writew(vsp, &dsp->vsp);
	writew(vdp, &dsp->vdp);

	writel(l2m, &dsp->l2m);
	writel(l2em, &dsp->l2em);
	writel(l2oa0, &dsp->l2oa0);
	writel(l2da0, &dsp->l2da0);
	writel(l2oa1, &dsp->l2oa1);
	writel(l2da1, &dsp->l2da1);
	writew(l2dx, &dsp->l2dx);
	writew(l2dy, &dsp->l2dy);
	writew(l2wx, &dsp->l2wx);
	writew(l2wy, &dsp->l2wy);
	writew(l2ww, &dsp->l2ww);
	writew(l2wh, &dsp->l2wh);

	writel(dcm1 | (1 << 18) | (1 << 31), &dsp->dcm1);
}

void *video_hw_init(void)
{
	struct mb86r0x_gdc *gdc = (struct mb86r0x_gdc *) MB86R0x_GDC_BASE;
	GraphicDevice *pGD = &mb86r0x;
	char *s;
	u32 *vid;

	memset(pGD, 0, sizeof(GraphicDevice));

	pGD->gdfIndex = GDF_15BIT_555RGB;
	pGD->gdfBytesPP = 2;
	pGD->memSize = VIDEO_MEM_SIZE;
	pGD->frameAdrs = PHYS_SDRAM + PHYS_SDRAM_SIZE - VIDEO_MEM_SIZE;

	vid = (u32 *)pGD->frameAdrs;

	s = getenv("videomode");
	if (s != NULL)
		dsp_init(&gdc->dsp0, s, vid);

	s = getenv("videomode1");
	if (s != NULL)
		dsp_init(&gdc->dsp1, s, vid);

	return pGD;
}
