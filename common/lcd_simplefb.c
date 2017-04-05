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
	int xsize, ysize;
	int bpix; /* log2 of bits per pixel */
	const char *name;
	ulong fb_base;

	xsize = lcd_get_pixel_width();
	ysize = lcd_get_pixel_height();
	bpix = LCD_BPP;
	fb_base = gd->fb_base;
	switch (bpix) {
	case 4: /* VIDEO_BPP16 */
		name = "r5g6b5";
		break;
	case 5: /* VIDEO_BPP32 */
		name = "a8r8g8b8";
		break;
	default:
		return -EINVAL;
	}

	return fdt_setup_simplefb_node(blob, off, fb_base, xsize, ysize,
				       xsize * (1 << bpix) / 8, name);
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
