/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * (C) Copyright 2015
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <video_font.h>		/* Get font data, width and height */

#define VIDEO_FONT_BYTE_WIDTH	((VIDEO_FONT_WIDTH / 8) + (VIDEO_FONT_WIDTH % 8 > 0))

#define FLIPPED_DIRECTION 1
#define NORMAL_DIRECTION 0

/**
 * Checks if bits per pixel supported.
 *
 * @param bpix	framebuffer bits per pixel.
 *
 * @returns 0, if supported, or else -ENOSYS.
 */
int check_bpix_support(int bpix);

/**
 * Fill 1 pixel in framebuffer, and go to next one.
 *
 * @param dstp		a pointer to pointer to framebuffer.
 * @param value		value to write to framebuffer.
 * @param pbytes	framebuffer bytes per pixel.
 * @param step		framebuffer pointer increment. Usually is equal to pbytes,
 *			and may be negative to control filling direction.
 */
void fill_pixel_and_goto_next(void **dstp, u32 value, int pbytes, int step);

/**
 * Fills 1 character in framebuffer vertically. Vertically means we're filling char font data rows
 * across the lines.
 *
 * @param pfont		a pointer to character font data.
 * @param line		a pointer to pointer to framebuffer. It's a point for upper left char corner
 * @param vid_priv	driver private data.
 * @param direction	controls character orientation. Can be normal or flipped.
 * When normal:               When flipped:
 *|-----------------------------------------------|
 *| line stepping        |                        |
 *|            |         |       stepping ->      |
 *|     *      |         |       * * *            |
 *|   * *      v         |         *              |
 *|     *                |         *              |
 *|     *                |         * *      ^     |
 *|   * * *              |         *        |     |
 *|                      |                  |     |
 *| stepping ->          |         line stepping  |
 *|---!!we're starting from upper left char corner|
 *|-----------------------------------------------|
 *
 * @returns 0, if success, or else error code.
 */
int fill_char_vertically(uchar *pfont, void **line, struct video_priv *vid_priv,
			 bool direction);

/**
 * Fills 1 character in framebuffer horizontally.
 * Horizontally means we're filling char font data columns across the lines.
 *
 * @param pfont		a pointer to character font data.
 * @param line		a pointer to pointer to framebuffer. It's a point for upper left char corner
 * @param vid_priv	driver private data.
 * @param direction	controls character orientation. Can be normal or flipped.
 * When normal:               When flipped:
 *|-----------------------------------------------|
 *|               *        |   line stepping      |
 *|    ^  * * * * *        |   |                  |
 *|    |    *     *        |   v   *     *        |
 *|    |                   |       * * * * *      |
 *|  line stepping         |       *              |
 *|                        |                      |
 *|  stepping ->           |        <- stepping   |
 *|---!!we're starting from upper left char corner|
 *|-----------------------------------------------|
 *
 * @returns 0, if success, or else error code.
 */
int fill_char_horizontally(uchar *pfont, void **line, struct video_priv *vid_priv,
			   bool direction);

/**
 * console probe function.
 *
 * @param dev	a pointer to device.
 *
 * @returns 0, if success, or else error code.
 */
int console_probe(struct udevice *dev);
