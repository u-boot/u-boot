/*
 * Copyright 2014 Google Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _DISPLAY_H
#define _DISPLAY_H

struct udevice;
struct display_timing;

/**
 * Display uclass platform data for each device
 *
 * @source_id:	ID for the source of the display data, typically a video
 * controller
 * @src_dev:	Source device providing the video
 */
struct display_plat {
	int source_id;
	struct udevice *src_dev;
};

/**
 * display_read_timing() - Read timing information from EDID
 *
 * @dev:	Device to read from
 * @return 0 if OK, -ve on error
 */
int display_read_timing(struct udevice *dev, struct display_timing *timing);

/**
 * display_port_enable() - Enable a display port device
 *
 * @dev:	Device to enable
 * @panel_bpp:	Number of bits per pixel for panel
 * @timing:	Display timings
 * @return 0 if OK, -ve on error
 */
int display_enable(struct udevice *dev, int panel_bpp,
		   const struct display_timing *timing);

struct dm_display_ops {
	/**
	 * read_edid() - Read information from EDID
	 *
	 * @dev:	Device to read from
	 * @buf:	Buffer to read into (should be EDID_SIZE bytes)
	 * @buf_size:	Buffer size (should be EDID_SIZE)
	 * @return number of bytes read, <=0 for error
	 */
	int (*read_edid)(struct udevice *dev, u8 *buf, int buf_size);

	/**
	 * enable() - Enable the display port device
	 *
	 * @dev:	Device to enable
	 * @panel_bpp:	Number of bits per pixel for panel
	 * @timing:	Display timings
	 * @return 0 if OK, -ve on error
	 */
	int (*enable)(struct udevice *dev, int panel_bpp,
		      const struct display_timing *timing);
};

#define display_get_ops(dev)	((struct dm_display_ops *)(dev)->driver->ops)

#endif
