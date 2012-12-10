/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Fabio Estevam <fabio.estevam@freescale.com>
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
#include <linux/list.h>
#include <asm/gpio.h>
#include <asm/arch/iomux.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define MX53LOCO_LCD_POWER		IMX_GPIO_NR(3, 24)

static struct fb_videomode const claa_wvga = {
	.name		= "CLAA07LC0ACW",
	.refresh	= 57,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 37037,
	.left_margin	= 40,
	.right_margin	= 60,
	.upper_margin	= 10,
	.lower_margin	= 10,
	.hsync_len	= 20,
	.vsync_len	= 10,
	.sync		= 0,
	.vmode		= FB_VMODE_NONINTERLACED
};

void setup_iomux_lcd(void)
{
	mxc_request_iomux(MX53_PIN_DI0_DISP_CLK, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DI0_PIN15, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DI0_PIN2, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DI0_PIN3, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT1, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT2, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT3, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT4, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT5, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT6, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT7, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT8, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT9, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT10, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT11, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT12, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT13, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT14, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT15, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT16, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT17, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT18, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT19, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT20, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT21, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT22, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_DISP0_DAT23, IOMUX_CONFIG_ALT0);

	/* Turn on GPIO backlight */
	mxc_request_iomux(MX53_PIN_EIM_D24, IOMUX_CONFIG_ALT1);
	gpio_direction_output(MX53LOCO_LCD_POWER, 1);

	/* Turn on display contrast */
	mxc_request_iomux(MX53_PIN_GPIO_1, IOMUX_CONFIG_ALT1);
	gpio_direction_output(IOMUX_TO_GPIO(MX53_PIN_GPIO_1), 1);
}

void lcd_enable(void)
{
	int ret = ipuv3_fb_init(&claa_wvga, 0, IPU_PIX_FMT_RGB565);
	if (ret)
		printf("LCD cannot be configured: %d\n", ret);
}
