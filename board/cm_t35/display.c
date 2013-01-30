/*
 * (C) Copyright 2012 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
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
 * Foundation, Inc.
 */
#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <asm/arch/dss.h>
#include <lcd.h>
#include <asm/arch-omap3/dss.h>

DECLARE_GLOBAL_DATA_PTR;

enum display_type {
	NONE,
	DVI,
};

#define CMAP_ADDR	0x80100000

/*
 * The frame buffer is allocated before we have the chance to parse user input.
 * To make sure enough memory is allocated for all resolutions, we define
 * vl_{col | row} to the maximal resolution supported by OMAP3.
 */
vidinfo_t panel_info = {
	.vl_col  = 1400,
	.vl_row  = 1050,
	.vl_bpix = LCD_BPP,
	.cmap = (ushort *)CMAP_ADDR,
};

static struct panel_config panel_cfg;
static enum display_type lcd_def;

/*
 * A note on DVI presets;
 * U-Boot can convert 8 bit BMP data to 16 bit BMP data, and OMAP DSS can
 * convert 16 bit data into 24 bit data. Thus, GFXFORMAT_RGB16 allows us to
 * support two BMP types with one setting.
 */
static const struct panel_config preset_dvi_640X480 = {
	.lcd_size	= PANEL_LCD_SIZE(640, 480),
	.timing_h	= DSS_HBP(48) | DSS_HFP(16) | DSS_HSW(96),
	.timing_v	= DSS_VBP(33) | DSS_VFP(10) | DSS_VSW(2),
	.divisor	= 12 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_800X600 = {
	.lcd_size	= PANEL_LCD_SIZE(800, 600),
	.timing_h	= DSS_HBP(88) | DSS_HFP(40) | DSS_HSW(128),
	.timing_v	= DSS_VBP(23) | DSS_VFP(1) | DSS_VSW(4),
	.divisor	= 8 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1024X768 = {
	.lcd_size	= PANEL_LCD_SIZE(1024, 768),
	.timing_h	= DSS_HBP(160) | DSS_HFP(24) | DSS_HSW(136),
	.timing_v	= DSS_VBP(29) | DSS_VFP(3) | DSS_VSW(6),
	.divisor	= 5 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1152X864 = {
	.lcd_size	= PANEL_LCD_SIZE(1152, 864),
	.timing_h	= DSS_HBP(256) | DSS_HFP(64) | DSS_HSW(128),
	.timing_v	= DSS_VBP(32) | DSS_VFP(1) | DSS_VSW(3),
	.divisor	= 3 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1280X960 = {
	.lcd_size	= PANEL_LCD_SIZE(1280, 960),
	.timing_h	= DSS_HBP(312) | DSS_HFP(96) | DSS_HSW(112),
	.timing_v	= DSS_VBP(36) | DSS_VFP(1) | DSS_VSW(3),
	.divisor	= 3 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

static const struct panel_config preset_dvi_1280X1024 = {
	.lcd_size	= PANEL_LCD_SIZE(1280, 1024),
	.timing_h	= DSS_HBP(248) | DSS_HFP(48) | DSS_HSW(112),
	.timing_v	= DSS_VBP(38) | DSS_VFP(1) | DSS_VSW(3),
	.divisor	= 3 | (1 << 16),
	.data_lines	= LCD_INTERFACE_24_BIT,
	.panel_type	= ACTIVE_DISPLAY,
	.load_mode	= 2,
	.gfx_format	= GFXFORMAT_RGB16,
};

/*
 * set_resolution_params()
 *
 * Due to usage of multiple display related APIs resolution data is located in
 * more than one place. This function updates them all.
 */
static void set_resolution_params(int x, int y)
{
	panel_cfg.lcd_size = PANEL_LCD_SIZE(x, y);
	panel_info.vl_col = x;
	panel_info.vl_row = y;
	lcd_line_length = (panel_info.vl_col * NBITS(panel_info.vl_bpix)) / 8;
}

static void set_preset(const struct panel_config preset, int x_res, int y_res)
{
	panel_cfg = preset;
	set_resolution_params(x_res, y_res);
}

static enum display_type set_dvi_preset(const struct panel_config preset,
					int x_res, int y_res)
{
	set_preset(preset, x_res, y_res);
	return DVI;
}

/*
 * env_parse_displaytype() - parse display type.
 *
 * Parses the environment variable "displaytype", which contains the
 * name of the display type or preset, in which case it applies its
 * configurations.
 *
 * Returns the type of display that was specified.
 */
static enum display_type env_parse_displaytype(char *displaytype)
{
	if (!strncmp(displaytype, "dvi640x480", 10))
		return set_dvi_preset(preset_dvi_640X480, 640, 480);
	else if (!strncmp(displaytype, "dvi800x600", 10))
		return set_dvi_preset(preset_dvi_800X600, 800, 600);
	else if (!strncmp(displaytype, "dvi1024x768", 11))
		return set_dvi_preset(preset_dvi_1024X768, 1024, 768);
	else if (!strncmp(displaytype, "dvi1152x864", 11))
		return set_dvi_preset(preset_dvi_1152X864, 1152, 864);
	else if (!strncmp(displaytype, "dvi1280x960", 11))
		return set_dvi_preset(preset_dvi_1280X960, 1280, 960);
	else if (!strncmp(displaytype, "dvi1280x1024", 12))
		return set_dvi_preset(preset_dvi_1280X1024, 1280, 1024);

	return NONE;
}

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;
void *lcd_base;
short console_col;
short console_row;
void *lcd_console_address;

void lcd_ctrl_init(void *lcdbase)
{
	struct prcm *prcm = (struct prcm *)PRCM_BASE;
	char *displaytype = getenv("displaytype");

	if (displaytype == NULL)
		return;

	lcd_def = env_parse_displaytype(displaytype);
	if (lcd_def == NONE)
		return;

	panel_cfg.frame_buffer = lcdbase;
	omap3_dss_panel_config(&panel_cfg);
	/*
	 * Pixel clock is defined with many divisions and only few
	 * multiplications of the system clock. Since DSS FCLK divisor is set
	 * to 16 by default, we need to set it to a smaller value, like 3
	 * (chosen via trial and error).
	 */
	clrsetbits_le32(&prcm->clksel_dss, 0xF, 3);
}

void lcd_enable(void)
{
	if (lcd_def == DVI) {
		gpio_direction_output(54, 0); /* Turn on DVI */
		omap3_dss_enable();
	}
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue) {}
