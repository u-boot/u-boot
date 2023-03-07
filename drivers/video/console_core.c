// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <video.h>
#include <video_console.h>
#include <dm.h>
#include "vidconsole_internal.h"

int check_bpix_support(int bpix)
{
	if (bpix == VIDEO_BPP8 && IS_ENABLED(CONFIG_VIDEO_BPP8))
		return 0;
	else if (bpix == VIDEO_BPP16 && IS_ENABLED(CONFIG_VIDEO_BPP16))
		return 0;
	else if (bpix == VIDEO_BPP32 && IS_ENABLED(CONFIG_VIDEO_BPP32))
		return 0;
	else
		return -ENOSYS;
}

inline void fill_pixel_and_goto_next(void **dstp, u32 value, int pbytes, int step)
{
	u8 *dst_byte = *dstp;

	if (pbytes == 4) {
		u32 *dst = *dstp;
		*dst = value;
	}
	if (pbytes == 2) {
		u16 *dst = *dstp;
		*dst = value;
	}
	if (pbytes == 1) {
		u8 *dst = *dstp;
		*dst = value;
	}
	*dstp = dst_byte + step;
}

int fill_char_vertically(uchar *pfont, void **line, struct video_priv *vid_priv,
			 bool direction)
{
	int step, line_step, pbytes, ret;
	void *dst;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	pbytes = VNBYTES(vid_priv->bpix);
	if (direction) {
		step = -pbytes;
		line_step = -vid_priv->line_length;
	} else {
		step = pbytes;
		line_step = vid_priv->line_length;
	}

	for (int row = 0; row < VIDEO_FONT_HEIGHT; row++) {
		dst = *line;
		uchar bits = pfont[row];

		for (int i = 0; i < VIDEO_FONT_WIDTH; i++) {
			u32 value = (bits & 0x80) ?
				vid_priv->colour_fg :
				vid_priv->colour_bg;

			fill_pixel_and_goto_next(&dst,
						 value,
						 pbytes,
						 step
			);
			bits <<= 1;
		}
		*line += line_step;
	}
	return ret;
}

int fill_char_horizontally(uchar *pfont, void **line, struct video_priv *vid_priv,
			   bool direction)
{
	int step, line_step, pbytes, ret;
	void *dst;
	u8 mask = 0x80;

	ret = check_bpix_support(vid_priv->bpix);
	if (ret)
		return ret;

	pbytes = VNBYTES(vid_priv->bpix);
	if (direction) {
		step = -pbytes;
		line_step = vid_priv->line_length;
	} else {
		step = pbytes;
		line_step = -vid_priv->line_length;
	}
	for (int col = 0; col < VIDEO_FONT_WIDTH; col++) {
		dst = *line;
		for (int row = 0; row < VIDEO_FONT_HEIGHT; row++) {
			u32 value = (pfont[row * VIDEO_FONT_BYTE_WIDTH] & mask) ?
						vid_priv->colour_fg :
						vid_priv->colour_bg;

			fill_pixel_and_goto_next(&dst,
						 value,
						 pbytes,
						 step
			);
		}
		*line += line_step;
		mask >>= 1;
	}
	return ret;
}

int console_probe(struct udevice *dev)
{
	struct vidconsole_priv *vc_priv = dev_get_uclass_priv(dev);
	struct udevice *vid_dev = dev->parent;
	struct video_priv *vid_priv = dev_get_uclass_priv(vid_dev);

	vc_priv->x_charsize = VIDEO_FONT_WIDTH;
	vc_priv->y_charsize = VIDEO_FONT_HEIGHT;
	if (vid_priv->rot % 2) {
		vc_priv->cols = vid_priv->ysize / VIDEO_FONT_WIDTH;
		vc_priv->rows = vid_priv->xsize / VIDEO_FONT_HEIGHT;
		vc_priv->xsize_frac = VID_TO_POS(vid_priv->ysize);
	} else {
		vc_priv->cols = vid_priv->xsize / VIDEO_FONT_WIDTH;
		vc_priv->rows = vid_priv->ysize / VIDEO_FONT_HEIGHT;
	}

	return 0;
}
