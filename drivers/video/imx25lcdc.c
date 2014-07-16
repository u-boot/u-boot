/*
 * (C) Copyright 2011
 * Matthias Weisser <weisserm@arcor.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * imx25lcdc.c - Graphic interface for i.MX25 lcd controller
 */

#include <common.h>

#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
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
static GraphicDevice imx25fb;

void *video_hw_init(void)
{
	struct lcdc_regs *lcdc = (struct lcdc_regs *)IMX_LCDC_BASE;
	struct ccm_regs *ccm = (struct ccm_regs *)IMX_CCM_BASE;
	GraphicDevice *pGD = &imx25fb;
	char *s;
	u32 *videomem;

	memset(pGD, 0, sizeof(GraphicDevice));

	pGD->gdfIndex = GDF_16BIT_565RGB;
	pGD->gdfBytesPP = 2;
	pGD->memSize = VIDEO_MEM_SIZE;
	pGD->frameAdrs = PHYS_SDRAM + PHYS_SDRAM_SIZE - VIDEO_MEM_SIZE;

	videomem = (u32 *)pGD->frameAdrs;

	s = getenv("videomode");
	if (s != NULL) {
		struct ctfb_res_modes var_mode;
		u32 lsr, lpcr, lhcr, lvcr;
		unsigned long div;
		int bpp;

		/* Disable all clocks of the LCDC */
		writel(readl(&ccm->cgr0) & ~((1<<7) | (1<<24)), &ccm->cgr0);
		writel(readl(&ccm->cgr1) & ~(1<<29), &ccm->cgr1);

		bpp = video_get_params(&var_mode, s);

		if (bpp == 0) {
			var_mode.xres = 320;
			var_mode.yres = 240;
			var_mode.pixclock = 154000;
			var_mode.left_margin = 68;
			var_mode.right_margin = 20;
			var_mode.upper_margin = 4;
			var_mode.lower_margin = 18;
			var_mode.hsync_len = 40;
			var_mode.vsync_len = 6;
			var_mode.sync = 0;
			var_mode.vmode = 0;
		}

		/* Fill memory with white */
		memset(videomem, 0xFF, var_mode.xres * var_mode.yres * 2);

		imx25fb.winSizeX = var_mode.xres;
		imx25fb.winSizeY = var_mode.yres;

		/* LCD base clock is 66.6MHZ. We do calculations in kHz */
		div = 66000 / (1000000000L / var_mode.pixclock);
		if (div > 63)
			div = 63;
		if (0 == div)
			div = 1;

		lsr = ((var_mode.xres / 16) << 20) |
			var_mode.yres;
		lpcr =	(1 << 31) |
			(1 << 30) |
			(5 << 25) |
			(1 << 23) |
			(1 << 22) |
			(1 << 19) |
			(1 <<  7) |
			div;
		lhcr =	(var_mode.right_margin << 0) |
			(var_mode.left_margin << 8) |
			(var_mode.hsync_len << 26);

		lvcr =	(var_mode.lower_margin << 0) |
			(var_mode.upper_margin << 8) |
			(var_mode.vsync_len << 26);

		writel((uint32_t)videomem, &lcdc->lssar);
		writel(lsr, &lcdc->lsr);
		writel(var_mode.xres * 2 / 4, &lcdc->lvpwr);
		writel(lpcr, &lcdc->lpcr);
		writel(lhcr, &lcdc->lhcr);
		writel(lvcr, &lcdc->lvcr);
		writel(0x00040060, &lcdc->ldcr);

		writel(0xA90300, &lcdc->lpccr);

		/* Ensable all clocks of the LCDC */
		writel(readl(&ccm->cgr0) | ((1<<7) | (1<<24)), &ccm->cgr0);
		writel(readl(&ccm->cgr1) | (1<<29), &ccm->cgr1);
	}

	return pGD;
}
