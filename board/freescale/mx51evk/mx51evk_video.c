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

#define MX51EVK_LCD_3V3		IMX_GPIO_NR(4, 9)
#define MX51EVK_LCD_5V		IMX_GPIO_NR(4, 10)
#define MX51EVK_LCD_BACKLIGHT	IMX_GPIO_NR(3, 4)

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
	/* DI2_PIN15 */
	mxc_request_iomux(MX51_PIN_DI_GP4, IOMUX_CONFIG_ALT4);

	/* Pad settings for MX51_PIN_DI2_DISP_CLK */
	mxc_iomux_set_pad(MX51_PIN_DI2_DISP_CLK, PAD_CTL_HYS_NONE |
			  PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_KEEPER |
			  PAD_CTL_DRV_MAX | PAD_CTL_SRE_SLOW);

	/* Turn on 3.3V voltage for LCD */
	mxc_request_iomux(MX51_PIN_CSI2_D12, IOMUX_CONFIG_ALT3);
	gpio_direction_output(MX51EVK_LCD_3V3, 1);

	/* Turn on 5V voltage for LCD */
	mxc_request_iomux(MX51_PIN_CSI2_D13, IOMUX_CONFIG_ALT3);
	gpio_direction_output(MX51EVK_LCD_5V, 1);

	/* Turn on GPIO backlight */
	mxc_request_iomux(MX51_PIN_DI1_D1_CS, IOMUX_CONFIG_ALT4);
	mxc_iomux_set_input(MX51_GPIO3_IPP_IND_G_IN_4_SELECT_INPUT,
							INPUT_CTL_PATH1);
	gpio_direction_output(MX51EVK_LCD_BACKLIGHT, 1);
}

void lcd_enable(void)
{
	int ret = ipuv3_fb_init(&claa_wvga, 1, IPU_PIX_FMT_RGB565);
	if (ret)
		printf("LCD cannot be configured: %d\n", ret);
}
