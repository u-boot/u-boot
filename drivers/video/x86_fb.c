/*
 *
 * Vesa frame buffer driver for x86
 *
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <video_fb.h>
#include <vbe.h>
#include "videomodes.h"

/*
 * The Graphic Device
 */
GraphicDevice ctfb;

void *video_hw_init(void)
{
	GraphicDevice *gdev = &ctfb;
	int bits_per_pixel;

	printf("Video: ");
	if (vbe_get_video_info(gdev)) {
		printf("No video mode configured\n");
		return NULL;
	}

	bits_per_pixel = gdev->gdfBytesPP * 8;
	sprintf(gdev->modeIdent, "%dx%dx%d", gdev->winSizeX, gdev->winSizeY,
		bits_per_pixel);
	printf("%s\n", gdev->modeIdent);

	return (void *)gdev;
}
