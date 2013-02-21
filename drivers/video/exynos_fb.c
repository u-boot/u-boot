/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/system.h>

#include "exynos_fb.h"

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;
void *lcd_console_address;

short console_col;
short console_row;

static unsigned int panel_width, panel_height;

static void exynos_lcd_init_mem(void *lcdbase, vidinfo_t *vid)
{
	unsigned long palette_size;
	unsigned int fb_size;

	fb_size = vid->vl_row * vid->vl_col * (NBITS(vid->vl_bpix) >> 3);

	lcd_base = lcdbase;

	palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;

	exynos_fimd_lcd_init_mem((unsigned long)lcd_base,
			(unsigned long)fb_size, palette_size);
}

static void exynos_lcd_init(vidinfo_t *vid)
{
	exynos_fimd_lcd_init(vid);

	/* Enable flushing after LCD writes if requested */
	lcd_set_flush_dcache(1);
}

#ifdef CONFIG_CMD_BMP
static void draw_logo(void)
{
	int x, y;
	ulong addr;

	if (panel_width >= panel_info.logo_width) {
		x = ((panel_width - panel_info.logo_width) >> 1);
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_height >= panel_info.logo_height) {
		y = ((panel_height - panel_info.logo_height) >> 1) - 4;
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	addr = panel_info.logo_addr;
	bmp_display(addr, x, y);
}
#endif

void __exynos_cfg_lcd_gpio(void)
{
}
void exynos_cfg_lcd_gpio(void)
	__attribute__((weak, alias("__exynos_cfg_lcd_gpio")));

void __exynos_backlight_on(unsigned int onoff)
{
}
void exynos_backlight_on(unsigned int onoff)
	__attribute__((weak, alias("__exynos_cfg_lcd_gpio")));

void __exynos_reset_lcd(void)
{
}
void exynos_reset_lcd(void)
	__attribute__((weak, alias("__exynos_reset_lcd")));

void __exynos_lcd_power_on(void)
{
}
void exynos_lcd_power_on(void)
	__attribute__((weak, alias("__exynos_lcd_power_on")));

void __exynos_cfg_ldo(void)
{
}
void exynos_cfg_ldo(void)
	__attribute__((weak, alias("__exynos_cfg_ldo")));

void __exynos_enable_ldo(unsigned int onoff)
{
}
void exynos_enable_ldo(unsigned int onoff)
	__attribute__((weak, alias("__exynos_enable_ldo")));

void __exynos_backlight_reset(void)
{
}
void exynos_backlight_reset(void)
	__attribute__((weak, alias("__exynos_backlight_reset")));

static void lcd_panel_on(vidinfo_t *vid)
{
	udelay(vid->init_delay);

	exynos_backlight_reset();

	exynos_cfg_lcd_gpio();

	exynos_lcd_power_on();

	udelay(vid->power_on_delay);

	if (vid->dp_enabled)
		exynos_init_dp();

	exynos_reset_lcd();

	udelay(vid->reset_delay);

	exynos_backlight_on(1);

	exynos_cfg_ldo();

	exynos_enable_ldo(1);

	if (vid->mipi_enabled)
		exynos_mipi_dsi_init();
}

void lcd_ctrl_init(void *lcdbase)
{
	set_system_display_ctrl();
	set_lcd_clk();

	/* initialize parameters which is specific to panel. */
	init_panel_info(&panel_info);

	panel_width = panel_info.vl_width;
	panel_height = panel_info.vl_height;

	exynos_lcd_init_mem(lcdbase, &panel_info);

	exynos_lcd_init(&panel_info);
}

void lcd_enable(void)
{
	if (panel_info.logo_on) {
		memset(lcd_base, 0, panel_width * panel_height *
				(NBITS(panel_info.vl_bpix) >> 3));
#ifdef CONFIG_CMD_BMP
		draw_logo();
#endif
	}

	lcd_panel_on(&panel_info);
}

/* dummy function */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}
