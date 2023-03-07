// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Get font data, width and height */
#include "vidconsole_internal.h"

static int console_set_row(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *line, *dst, *end;
	int pixels = VIDEO_FONT_HEIGHT * vid_priv->xsize;
	int ret;
	int i;
	int pbytes;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	line = vid_priv->fb + row * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	dst = line;
	pbytes = VNBYTES(vid_priv->bpix);
	for (i = 0; i < pixels; i++)
		fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
	end = dst;

	ret = vidconsole_sync_copy(dev, line, end);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows(struct udevice *dev, uint rowdst,
			     uint rowsrc, uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *dst;
	void *src;
	int size;
	int ret;

	dst = vid_priv->fb + rowdst * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	src = vid_priv->fb + rowsrc * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	size = VIDEO_FONT_HEIGHT * vid_priv->line_length * count;
	ret = vidconsole_memmove(dev, dst, src, size);
	if (ret)
		return ret;

	return 0;
}

static int console_putc_xy(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	int pbytes = VNBYTES(vid_priv->bpix);
	int x, linenum, ret;
	void *start, *line;
	uchar *pfont = video_fontdata + (u8)ch * VIDEO_FONT_CHAR_PIXEL_BYTES;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	linenum = y;
	x = VID_TO_PIXEL(x_frac);
	start = vid_priv->fb + linenum * vid_priv->line_length + x * pbytes;
	line = start;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;

	ret = fill_char_vertically(pfont, &line, vid_priv, NORMAL_DIRECTION);
	if (ret)
		return ret;

	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return VID_TO_POS(VIDEO_FONT_WIDTH);
}

struct vidconsole_ops console_ops = {
	.putc_xy	= console_putc_xy,
	.move_rows	= console_move_rows,
	.set_row	= console_set_row,
};

U_BOOT_DRIVER(vidconsole_normal) = {
	.name	= "vidconsole0",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops,
	.probe	= console_probe,
};
