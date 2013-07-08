/*
 * (C) Copyright 2003 Wolfgang Grandegger <wg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _VIDEO_AD7179_H_
#define _VIDEO_AD7179_H_

/*
 * The video encoder data are board specific now!
 */

#if defined(CONFIG_RRVISION)
#include "../board/RRvision/video_ad7179.h"
#else
#error "Please provide a board-specific video_ad7179.h"
#endif

#endif /* _VIDEO_AD7179_H_ */
