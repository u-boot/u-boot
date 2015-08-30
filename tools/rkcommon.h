/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _RKCOMMON_H
#define _RKCOMMON_H

enum {
	RK_BLK_SIZE		= 512,
	RK_CODE1_OFFSET		= 4,
	RK_MAX_CODE1_SIZE	= 32 << 10,
};

/**
 * rkcommon_set_header() - set up the header for a Rockchip boot image
 *
 * This sets up a 2KB header which can be interpreted by the Rockchip boot ROM.
 *
 * @buf:	Pointer to header place (must be at least 2KB in size)
 * @file_size:	Size of the file we want the boot ROM to load, in bytes
 * @return 0 if OK, -ENOSPC if too large
 */
int rkcommon_set_header(void *buf, uint file_size);

#endif
