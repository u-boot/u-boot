/*
 * Simplefb device tree support
 *
 * (C) Copyright 2015
 * Stephen Warren <swarren@wwwdotorg.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <fdt_support.h>
#include <libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

static int lcd_dt_simplefb_configure_node(void *blob, int off)
{
#if LCD_BPP == LCD_COLOR16
	int vl_col = lcd_get_pixel_width();
	int vl_row = lcd_get_pixel_height();
	return fdt_setup_simplefb_node(blob, off, gd->fb_base, vl_col, vl_row,
				       vl_col * 2, "r5g6b5");
#else
	return -1;
#endif
}

int lcd_dt_simplefb_add_node(void *blob)
{
	static const char compat[] = "simple-framebuffer";
	static const char disabled[] = "disabled";
	int off, ret;

	off = fdt_add_subnode(blob, 0, "framebuffer");
	if (off < 0)
		return -1;

	ret = fdt_setprop(blob, off, "status", disabled, sizeof(disabled));
	if (ret < 0)
		return -1;

	ret = fdt_setprop(blob, off, "compatible", compat, sizeof(compat));
	if (ret < 0)
		return -1;

	return lcd_dt_simplefb_configure_node(blob, off);
}

int lcd_dt_simplefb_enable_existing_node(void *blob)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, "simple-framebuffer");
	if (off < 0)
		return -1;

	return lcd_dt_simplefb_configure_node(blob, off);
}
