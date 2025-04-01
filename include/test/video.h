/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc.
 */

#ifndef __TEST_VIDEO_H
#define __TEST_VIDEO_H

#include <stdbool.h>

struct udevice;
struct unit_test_state;

/**
 * video_compress_fb() - Compress the frame buffer and return its size
 *
 * We want to write tests which perform operations on the video console and
 * check that the frame buffer ends up with the correct contents. But it is
 * painful to store 'known good' images for comparison with the frame
 * buffer. As an alternative, we can compress the frame buffer and check the
 * size of the compressed data. This provides a pretty good level of
 * certainty and the resulting tests need only check a single value.
 *
 * @uts:	Test state
 * @dev:	Video device
 * @use_copy:	Use copy frame buffer if available
 * Return: compressed size of the frame buffer, or -ve on error
 */
int video_compress_fb(struct unit_test_state *uts, struct udevice *dev,
		      bool use_copy);

/**
 * check_copy_frame_buffer() - Compare main frame buffer to copy
 *
 * If the copy frame buffer is enabled, this compares it to the main
 * frame buffer. Normally they should have the same contents after a
 * sync.
 *
 * @uts:	Test state
 * @dev:	Video device
 * Return: 0, or -ve on error
 */
int video_check_copy_fb(struct unit_test_state *uts, struct udevice *dev);

#endif
