/*
 * S3C24x0 LCD driver
 *
 * NOTE: Only 16/24 bpp operation with TFT LCD is supported.
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <malloc.h>
#include <video_fb.h>

#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>

#include "videomodes.h"

static GraphicDevice panel;

/* S3C requires the FB to be 4MiB aligned. */
#define S3CFB_ALIGN			(4 << 20)

#define S3CFB_LCDCON1_CLKVAL(x)		((x) << 8)
#define S3CFB_LCDCON1_PNRMODE_TFT	(0x3 << 5)
#define S3CFB_LCDCON1_BPPMODE_TFT_16BPP	(0xc << 1)
#define S3CFB_LCDCON1_BPPMODE_TFT_24BPP	(0xd << 1)

#define S3CFB_LCDCON2_VBPD(x)		((x) << 24)
#define S3CFB_LCDCON2_LINEVAL(x)	((x) << 14)
#define S3CFB_LCDCON2_VFPD(x)		((x) << 6)
#define S3CFB_LCDCON2_VSPW(x)		((x) << 0)

#define S3CFB_LCDCON3_HBPD(x)		((x) << 19)
#define S3CFB_LCDCON3_HOZVAL(x)		((x) << 8)
#define S3CFB_LCDCON3_HFPD(x)		((x) << 0)

#define S3CFB_LCDCON4_HSPW(x)		((x) << 0)

#define S3CFB_LCDCON5_BPP24BL		(1 << 12)
#define S3CFB_LCDCON5_FRM565		(1 << 11)
#define S3CFB_LCDCON5_HWSWP		(1 << 0)

#define	PS2KHZ(ps)			(1000000000UL / (ps))

/*
 * Example:
 * setenv videomode video=ctfb:x:800,y:480,depth:16,mode:0,\
 *            pclk:30066,le:41,ri:89,up:45,lo:12,
 *            hs:1,vs:1,sync:100663296,vmode:0
 */
static void s3c_lcd_init(GraphicDevice *panel,
			struct ctfb_res_modes *mode, int bpp)
{
	uint32_t clk_divider;
	struct s3c24x0_lcd *regs = s3c24x0_get_base_lcd();

	/* Stop the controller. */
	clrbits_le32(&regs->lcdcon1, 1);

	/* Calculate clock divider. */
	clk_divider = (get_HCLK() / PS2KHZ(mode->pixclock)) / 1000;
	clk_divider = DIV_ROUND_UP(clk_divider, 2);
	if (clk_divider)
		clk_divider -= 1;

	/* Program LCD configuration. */
	switch (bpp) {
	case 16:
		writel(S3CFB_LCDCON1_BPPMODE_TFT_16BPP |
		       S3CFB_LCDCON1_PNRMODE_TFT |
		       S3CFB_LCDCON1_CLKVAL(clk_divider),
		       &regs->lcdcon1);
		writel(S3CFB_LCDCON5_HWSWP | S3CFB_LCDCON5_FRM565,
		       &regs->lcdcon5);
		break;
	case 24:
		writel(S3CFB_LCDCON1_BPPMODE_TFT_24BPP |
		       S3CFB_LCDCON1_PNRMODE_TFT |
		       S3CFB_LCDCON1_CLKVAL(clk_divider),
		       &regs->lcdcon1);
		writel(S3CFB_LCDCON5_BPP24BL, &regs->lcdcon5);
		break;
	}

	writel(S3CFB_LCDCON2_LINEVAL(mode->yres - 1) |
	       S3CFB_LCDCON2_VBPD(mode->upper_margin - 1) |
	       S3CFB_LCDCON2_VFPD(mode->lower_margin - 1) |
	       S3CFB_LCDCON2_VSPW(mode->vsync_len - 1),
	       &regs->lcdcon2);

	writel(S3CFB_LCDCON3_HBPD(mode->right_margin - 1) |
	       S3CFB_LCDCON3_HFPD(mode->left_margin - 1) |
	       S3CFB_LCDCON3_HOZVAL(mode->xres - 1),
	       &regs->lcdcon3);

	writel(S3CFB_LCDCON4_HSPW(mode->hsync_len - 1),
	       &regs->lcdcon4);

	/* Write FB address. */
	writel(panel->frameAdrs >> 1, &regs->lcdsaddr1);
	writel((panel->frameAdrs +
	       (mode->xres * mode->yres * panel->gdfBytesPP)) >> 1,
	       &regs->lcdsaddr2);
	writel(mode->xres * bpp / 16, &regs->lcdsaddr3);

	/* Start the controller. */
	setbits_le32(&regs->lcdcon1, 1);
}

void *video_hw_init(void)
{
	int bpp = -1;
	char *penv;
	void *fb;
	struct ctfb_res_modes mode;

	puts("Video: ");

	/* Suck display configuration from "videomode" variable */
	penv = getenv("videomode");
	if (!penv) {
		puts("S3CFB: 'videomode' variable not set!\n");
		return NULL;
	}

	bpp = video_get_params(&mode, penv);

	/* fill in Graphic device struct */
	sprintf(panel.modeIdent, "%dx%dx%d", mode.xres, mode.yres, bpp);

	panel.winSizeX = mode.xres;
	panel.winSizeY = mode.yres;
	panel.plnSizeX = mode.xres;
	panel.plnSizeY = mode.yres;

	switch (bpp) {
	case 24:
		panel.gdfBytesPP = 4;
		panel.gdfIndex = GDF_32BIT_X888RGB;
		break;
	case 16:
		panel.gdfBytesPP = 2;
		panel.gdfIndex = GDF_16BIT_565RGB;
		break;
	default:
		printf("S3CFB: Invalid BPP specified! (bpp = %i)\n", bpp);
		return NULL;
	}

	panel.memSize = mode.xres * mode.yres * panel.gdfBytesPP;

	/* Allocate framebuffer */
	fb = memalign(S3CFB_ALIGN, roundup(panel.memSize, S3CFB_ALIGN));
	if (!fb) {
		printf("S3CFB: Error allocating framebuffer!\n");
		return NULL;
	}

	/* Wipe framebuffer */
	memset(fb, 0, panel.memSize);

	panel.frameAdrs = (u32)fb;

	printf("%s\n", panel.modeIdent);

	/* Start framebuffer */
	s3c_lcd_init(&panel, &mode, bpp);

	return (void *)&panel;
}
