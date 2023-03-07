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

static int console_set_row_1(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	int pbytes = VNBYTES(vid_priv->bpix);
	void *start, *dst, *line;
	int i, j;
	int ret;

	start = vid_priv->fb + vid_priv->line_length -
		(row + 1) * VIDEO_FONT_HEIGHT * pbytes;
	line = start;
	for (j = 0; j < vid_priv->ysize; j++) {
		dst = line;
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
		line += vid_priv->line_length;
	}
	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows_1(struct udevice *dev, uint rowdst, uint rowsrc,
				   uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	int pbytes = VNBYTES(vid_priv->bpix);
	void *dst;
	void *src;
	int j, ret;

	dst = vid_priv->fb + vid_priv->line_length -
		(rowdst + count) * VIDEO_FONT_HEIGHT * pbytes;
	src = vid_priv->fb + vid_priv->line_length -
		(rowsrc + count) * VIDEO_FONT_HEIGHT * pbytes;

	for (j = 0; j < vid_priv->ysize; j++) {
		ret = vidconsole_memmove(dev, dst, src,
					VIDEO_FONT_HEIGHT * pbytes * count);
		if (ret)
			return ret;
		src += vid_priv->line_length;
		dst += vid_priv->line_length;
	}

	return 0;
}

static int console_putc_xy_1(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	int pbytes = VNBYTES(vid_priv->bpix);
	int x, linenum, ret;
	void *start, *line;
	uchar *pfont = video_fontdata + (u8)ch * VIDEO_FONT_HEIGHT;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	linenum = VID_TO_PIXEL(x_frac) + 1;
	x = y + 1;
	start = vid_priv->fb + linenum * vid_priv->line_length - x * pbytes;
	line = start;

	ret = fill_char_horizontally(pfont, &line, vid_priv, FLIPPED_DIRECTION);
	if (ret)
		return ret;

	/* We draw backwards from 'start, so account for the first line */
	ret = vidconsole_sync_copy(dev, start - vid_priv->line_length, line);
	if (ret)
		return ret;

	return VID_TO_POS(VIDEO_FONT_WIDTH);
}


static int console_set_row_2(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *start, *line, *dst, *end;
	int pixels = VIDEO_FONT_HEIGHT * vid_priv->xsize;
	int i, ret;
	int pbytes = VNBYTES(vid_priv->bpix);

	start = vid_priv->fb + vid_priv->ysize * vid_priv->line_length -
		(row + 1) * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	line = start;
	dst = line;
	for (i = 0; i < pixels; i++)
		fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
	end = dst;
	ret = vidconsole_sync_copy(dev, start, end);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows_2(struct udevice *dev, uint rowdst, uint rowsrc,
			       uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *dst;
	void *src;
	void *end;

	end = vid_priv->fb + vid_priv->ysize * vid_priv->line_length;
	dst = end - (rowdst + count) * VIDEO_FONT_HEIGHT *
		vid_priv->line_length;
	src = end - (rowsrc + count) * VIDEO_FONT_HEIGHT *
		vid_priv->line_length;
	vidconsole_memmove(dev, dst, src,
			   VIDEO_FONT_HEIGHT * vid_priv->line_length * count);

	return 0;
}

static int console_putc_xy_2(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	int pbytes = VNBYTES(vid_priv->bpix);
	int linenum, x, ret;
	void *start, *line;
	uchar *pfont = video_fontdata + (u8)ch * VIDEO_FONT_HEIGHT;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	linenum = vid_priv->ysize - y - 1;
	x = vid_priv->xsize - VID_TO_PIXEL(x_frac) - 1;
	start = vid_priv->fb + linenum * vid_priv->line_length + x * pbytes;
	line = start;

	ret = fill_char_vertically(pfont, &line, vid_priv, FLIPPED_DIRECTION);
	if (ret)
		return ret;

	/* Add 4 bytes to allow for the first pixel writen */
	ret = vidconsole_sync_copy(dev, start + 4, line);
	if (ret)
		return ret;

	return VID_TO_POS(VIDEO_FONT_WIDTH);
}

static int console_set_row_3(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	int pbytes = VNBYTES(vid_priv->bpix);
	void *start, *dst, *line;
	int i, j, ret;

	start = vid_priv->fb + row * VIDEO_FONT_HEIGHT * pbytes;
	line = start;
	for (j = 0; j < vid_priv->ysize; j++) {
		dst = line;
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
			fill_pixel_and_goto_next(&dst, clr, pbytes, pbytes);
		line += vid_priv->line_length;
	}
	ret = vidconsole_sync_copy(dev, start, line);
	if (ret)
		return ret;

	return 0;
}

static int console_move_rows_3(struct udevice *dev, uint rowdst, uint rowsrc,
			       uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	int pbytes = VNBYTES(vid_priv->bpix);
	void *dst;
	void *src;
	int j, ret;

	dst = vid_priv->fb + rowdst * VIDEO_FONT_HEIGHT * pbytes;
	src = vid_priv->fb + rowsrc * VIDEO_FONT_HEIGHT * pbytes;

	for (j = 0; j < vid_priv->ysize; j++) {
		ret = vidconsole_memmove(dev, dst, src,
					 VIDEO_FONT_HEIGHT * pbytes * count);
		if (ret)
			return ret;
		src += vid_priv->line_length;
		dst += vid_priv->line_length;
	}

	return 0;
}

static int console_putc_xy_3(struct udevice *dev, uint x_frac, uint y, char ch)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	int pbytes = VNBYTES(vid_priv->bpix);
	int linenum, x, ret;
	void *start, *line;
	uchar *pfont = video_fontdata + (u8)ch * VIDEO_FONT_HEIGHT;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	x = y;
	linenum = vid_priv->ysize - VID_TO_PIXEL(x_frac) - 1;
	start = vid_priv->fb + linenum * vid_priv->line_length + y * pbytes;
	line = start;

	ret = fill_char_horizontally(pfont, &line, vid_priv, NORMAL_DIRECTION);
	if (ret)
		return ret;
	/* Add a line to allow for the first pixels writen */
	ret = vidconsole_sync_copy(dev, start + vid_priv->line_length, line);
	if (ret)
		return ret;

	return VID_TO_POS(VIDEO_FONT_WIDTH);
}

struct vidconsole_ops console_ops_1 = {
	.putc_xy	= console_putc_xy_1,
	.move_rows	= console_move_rows_1,
	.set_row	= console_set_row_1,
};

struct vidconsole_ops console_ops_2 = {
	.putc_xy	= console_putc_xy_2,
	.move_rows	= console_move_rows_2,
	.set_row	= console_set_row_2,
};

struct vidconsole_ops console_ops_3 = {
	.putc_xy	= console_putc_xy_3,
	.move_rows	= console_move_rows_3,
	.set_row	= console_set_row_3,
};

U_BOOT_DRIVER(vidconsole_1) = {
	.name	= "vidconsole1",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_1,
	.probe	= console_probe,
};

U_BOOT_DRIVER(vidconsole_2) = {
	.name	= "vidconsole2",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_2,
	.probe	= console_probe,
};

U_BOOT_DRIVER(vidconsole_3) = {
	.name	= "vidconsole3",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_3,
	.probe	= console_probe,
};
