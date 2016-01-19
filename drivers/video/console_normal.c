/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2001-2015
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>
#include <video_font.h>		/* Get font data, width and height */

static int console_normal_set_row(struct udevice *dev, uint row, int clr)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *line;
	int pixels = VIDEO_FONT_HEIGHT * vid_priv->line_length;
	int i;

	line = vid_priv->fb + row * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	switch (vid_priv->bpix) {
#ifdef CONFIG_VIDEO_BPP8
	case VIDEO_BPP8: {
		uint8_t *dst = line;

		for (i = 0; i < pixels; i++)
			*dst++ = clr;
		break;
	}
#endif
#ifdef CONFIG_VIDEO_BPP16
	case VIDEO_BPP16: {
		uint16_t *dst = line;

		for (i = 0; i < pixels; i++)
			*dst++ = clr;
		break;
	}
#endif
#ifdef CONFIG_VIDEO_BPP32
	case VIDEO_BPP32: {
		uint32_t *dst = line;

		for (i = 0; i < pixels; i++)
			*dst++ = clr;
		break;
	}
#endif
	default:
		return -ENOSYS;
	}

	return 0;
}

static int console_normal_move_rows(struct udevice *dev, uint rowdst,
				     uint rowsrc, uint count)
{
	struct video_priv *vid_priv = dev_get_uclass_priv(dev->parent);
	void *dst;
	void *src;

	dst = vid_priv->fb + rowdst * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	src = vid_priv->fb + rowsrc * VIDEO_FONT_HEIGHT * vid_priv->line_length;
	memmove(dst, src, VIDEO_FONT_HEIGHT * vid_priv->line_length * count);

	return 0;
}

static int console_normal_putc_xy(struct udevice *dev, uint x, uint y, char ch)
{
	struct udevice *vid = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid);
	int i, row;
	void *line = vid_priv->fb + y * vid_priv->line_length +
		x * VNBYTES(vid_priv->bpix);

	for (row = 0; row < VIDEO_FONT_HEIGHT; row++) {
		uchar bits = video_fontdata[ch * VIDEO_FONT_HEIGHT + row];

		switch (vid_priv->bpix) {
#ifdef CONFIG_VIDEO_BPP8
		case VIDEO_BPP8: {
			uint8_t *dst = line;

			for (i = 0; i < VIDEO_FONT_WIDTH; i++) {
				*dst++ = (bits & 0x80) ? vid_priv->colour_fg
					: vid_priv->colour_bg;
				bits <<= 1;
			}
			break;
		}
#endif
#ifdef CONFIG_VIDEO_BPP16
		case VIDEO_BPP16: {
			uint16_t *dst = line;

			for (i = 0; i < VIDEO_FONT_WIDTH; i++) {
				*dst++ = (bits & 0x80) ? vid_priv->colour_fg
					: vid_priv->colour_bg;
				bits <<= 1;
			}
			break;
		}
#endif
#ifdef CONFIG_VIDEO_BPP32
		case VIDEO_BPP32: {
			uint32_t *dst = line;

			for (i = 0; i < VIDEO_FONT_WIDTH; i++) {
				*dst++ = (bits & 0x80) ? vid_priv->colour_fg
					: vid_priv->colour_bg;
				bits <<= 1;
			}
			break;
		}
#endif
		default:
			return -ENOSYS;
		}
		line += vid_priv->line_length;
	}

	return 0;
}

struct vidconsole_ops console_normal_ops = {
	.putc_xy	= console_normal_putc_xy,
	.move_rows	= console_normal_move_rows,
	.set_row	= console_normal_set_row,
};

U_BOOT_DRIVER(vidconsole_normal) = {
	.name	= "vidconsole0",
	.id	= UCLASS_VIDEO_CONSOLE,
	.ops	= &console_normal_ops,
};
