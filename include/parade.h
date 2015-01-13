/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PARADE_H__
#define __PARADE_H__

/* Initialize the Parade dP<->LVDS bridge if present */
#ifdef CONFIG_VIDEO_PARADE
int parade_init(const void *blob);
#else
static inline int parade_init(const void *blob) { return -1; }
#endif

#endif	/* __PARADE_H__ */
