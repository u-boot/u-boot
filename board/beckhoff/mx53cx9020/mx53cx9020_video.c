/*
 * Copyright (C) 2015  Beckhoff Automation GmbH & Co. KG
 * Patrick Bruenn <p.bruenn@beckhoff.com>
 *
 * Based on <u-boot>/board/freescale/mx53loco/mx53loco_video.c
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/list.h>
#include <asm/gpio.h>
#include <asm/arch/iomux-mx53.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define CX9020_DVI_PWD	IMX_GPIO_NR(6, 1)

static struct fb_videomode const vga_640x480 = {
	.name = "VESA_VGA_640x480",
	.refresh = 60,
	.xres = 640,
	.yres = 480,
	.pixclock = 39721,	/* picosecond (25.175 MHz) */
	.left_margin = 40,
	.right_margin = 60,
	.upper_margin = 10,
	.lower_margin = 10,
	.hsync_len = 20,
	.vsync_len = 10,
	.sync = 0,
	.vmode = FB_VMODE_NONINTERLACED
};

void setup_iomux_lcd(void)
{
	/* Turn on DVI_PWD */
	imx_iomux_v3_setup_pad(MX53_PAD_CSI0_DAT15__GPIO6_1);
	gpio_direction_output(CX9020_DVI_PWD, 1);
}

int board_video_skip(void)
{
	const int ret = ipuv3_fb_init(&vga_640x480, 0, IPU_PIX_FMT_RGB24);
	if (ret)
		printf("VESA VG 640x480 cannot be configured: %d\n", ret);
	return ret;
}
