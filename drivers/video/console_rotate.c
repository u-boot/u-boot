// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Get font data, width and height */

static int console_set_row_1(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	int pbytes = VNBYTES(vid_priv->bpix);
	void *start, *line;
	int i, j;
	int ret;

	start = vid_priv->fb + vid_priv->line_length -
		(row + 1) * VIDEO_FONT_HEIGHT * pbytes;
	line = start;
	for (j = 0; j < vid_priv->ysize; j++) {
		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				uint8_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
					*dst++ = clr;
				break;
			}
		case VIDEO_BPP16:
			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				uint16_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
					*dst++ = clr;
				break;
			}
		case VIDEO_BPP32:
			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				uint32_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
					*dst++ = clr;
				break;
			}
		default:
			return -ENOSYS;
		}
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
	uchar *pfont = video_fontdata + (u8)ch * VIDEO_FONT_HEIGHT;
	int pbytes = VNBYTES(vid_priv->bpix);
	int i, col, x, linenum, ret;
	int mask = 0x80;
	void *start, *line;

	linenum = VID_TO_PIXEL(x_frac) + 1;
	x = y + 1;
	start = vid_priv->fb + linenum * vid_priv->line_length - x * pbytes;
	line = start;
	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;

	for (col = 0; col < VIDEO_FONT_HEIGHT; col++) {
		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				uint8_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
					*dst-- = (pfont[i] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
				}
				break;
			}
		case VIDEO_BPP16:
			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				uint16_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
					*dst-- = (pfont[i] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
				}
				break;
			}
		case VIDEO_BPP32:
			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				uint32_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
					*dst-- = (pfont[i] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
				}
				break;
			}
		default:
			return -ENOSYS;
		}
		line += vid_priv->line_length;
		mask >>= 1;
	}
	/* We draw backwards from 'start, so account for the first line */
	ret = vidconsole_sync_copy(dev, start - vid_priv->line_length, line);
	if (ret)
		return ret;

	return VID_TO_POS(VIDEO_FONT_WIDTH);
}


static int console_set_row_2(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *start, *line, *end;
	int pixels = VIDEO_FONT_HEIGHT * vid_priv->xsize;
	int i, ret;

	start = vid_priv->fb + vid_priv->ysize * vid_priv->line_length -
		(row + 1) * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	line = start;
	switch (vid_priv->bpix) {
	case VIDEO_BPP8:
		if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
			uint8_t *dst = line;

			for (i = 0; i < pixels; i++)
				*dst++ = clr;
			end = dst;
			break;
		}
	case VIDEO_BPP16:
		if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
			uint16_t *dst = line;

			for (i = 0; i < pixels; i++)
				*dst++ = clr;
			end = dst;
			break;
		}
	case VIDEO_BPP32:
		if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
			uint32_t *dst = line;

			for (i = 0; i < pixels; i++)
				*dst++ = clr;
			end = dst;
			break;
		}
	default:
		return -ENOSYS;
	}
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
	int i, row, x, linenum, ret;
	void *start, *line;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	linenum = vid_priv->ysize - y - 1;
	x = vid_priv->xsize - VID_TO_PIXEL(x_frac) - 1;
	start = vid_priv->fb + linenum * vid_priv->line_length + x * pbytes;
	line = start;

	for (row = 0; row < VIDEO_FONT_HEIGHT; row++) {
		unsigned int idx = (u8)ch * VIDEO_FONT_HEIGHT + row;
		uchar bits = video_fontdata[idx];

		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				uint8_t *dst = line;

				for (i = 0; i < VIDEO_FONT_WIDTH; i++) {
					*dst-- = (bits & 0x80) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
					bits <<= 1;
				}
				break;
			}
		case VIDEO_BPP16:
			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				uint16_t *dst = line;

				for (i = 0; i < VIDEO_FONT_WIDTH; i++) {
					*dst-- = (bits & 0x80) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
					bits <<= 1;
				}
				break;
			}
		case VIDEO_BPP32:
			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				uint32_t *dst = line;

				for (i = 0; i < VIDEO_FONT_WIDTH; i++) {
					*dst-- = (bits & 0x80) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
					bits <<= 1;
				}
				break;
			}
		default:
			return -ENOSYS;
		}
		line -= vid_priv->line_length;
	}
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
	void *start, *line;
	int i, j, ret;

	start = vid_priv->fb + row * VIDEO_FONT_HEIGHT * pbytes;
	line = start;
	for (j = 0; j < vid_priv->ysize; j++) {
		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				uint8_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
					*dst++ = clr;
				break;
			}
		case VIDEO_BPP16:
			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				uint16_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
					*dst++ = clr;
				break;
			}
		case VIDEO_BPP32:
			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				uint32_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++)
					*dst++ = clr;
				break;
			}
		default:
			return -ENOSYS;
		}
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
	uchar *pfont = video_fontdata + (u8)ch * VIDEO_FONT_HEIGHT;
	int pbytes = VNBYTES(vid_priv->bpix);
	int i, col, x, ret;
	int mask = 0x80;
	void *start, *line;

	if (x_frac + VID_TO_POS(vc_priv->x_charsize) > vc_priv->xsize_frac)
		return -EAGAIN;
	x = vid_priv->ysize - VID_TO_PIXEL(x_frac) - 1;
	start = vid_priv->fb + x * vid_priv->line_length + y * pbytes;
	line = start;
	for (col = 0; col < VIDEO_FONT_HEIGHT; col++) {
		switch (vid_priv->bpix) {
		case VIDEO_BPP8:
			if (IS_ENABLED(CONFIG_VIDEO_BPP8)) {
				uint8_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
					*dst++ = (pfont[i] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
				}
				break;
			}
		case VIDEO_BPP16:
			if (IS_ENABLED(CONFIG_VIDEO_BPP16)) {
				uint16_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
					*dst++ = (pfont[i] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
				}
				break;
			}
		case VIDEO_BPP32:
			if (IS_ENABLED(CONFIG_VIDEO_BPP32)) {
				uint32_t *dst = line;

				for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
					*dst++ = (pfont[i] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;
				}
				break;
			}
		default:
			return -ENOSYS;
		}
		line -= vid_priv->line_length;
		mask >>= 1;
	}
	/* Add a line to allow for the first pixels writen */
	ret = vidconsole_sync_copy(dev, start + vid_priv->line_length, line);
	if (ret)
		return ret;

	return VID_TO_POS(VIDEO_FONT_WIDTH);
}


static int console_probe_2(struct udevice *dev)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);

	vc_priv->x_charsize = VIDEO_FONT_WIDTH;
	vc_priv->y_charsize = VIDEO_FONT_HEIGHT;
	vc_priv->cols = vid_priv->xsize / VIDEO_FONT_WIDTH;
	vc_priv->rows = vid_priv->ysize / VIDEO_FONT_HEIGHT;

	return 0;
}

static int console_probe_1_3(struct udevice *dev)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);

	vc_priv->x_charsize = VIDEO_FONT_WIDTH;
	vc_priv->y_charsize = VIDEO_FONT_HEIGHT;
	vc_priv->cols = vid_priv->ysize / VIDEO_FONT_WIDTH;
	vc_priv->rows = vid_priv->xsize / VIDEO_FONT_HEIGHT;
	vc_priv->xsize_frac = VID_TO_POS(vid_priv->ysize);

	return 0;
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
	.probe	= console_probe_1_3,
};

U_BOOT_DRIVER(vidconsole_2) = {
	.name	= "vidconsole2",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_2,
	.probe	= console_probe_2,
};

U_BOOT_DRIVER(vidconsole_3) = {
	.name	= "vidconsole3",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_ops_3,
	.probe	= console_probe_1_3,
};
