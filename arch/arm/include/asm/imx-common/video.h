/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IMX_VIDEO_H_
#define __IMX_VIDEO_H_

#include <linux/fb.h>
#include <ipu_pixfmt.h>

struct display_info_t {
	int	bus;
	int	addr;
	int	pixfmt;
	int	(*detect)(struct display_info_t const *dev);
	void	(*enable)(struct display_info_t const *dev);
	struct	fb_videomode mode;
};

#endif
