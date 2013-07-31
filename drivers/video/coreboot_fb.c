/*
 * coreboot Framebuffer driver.
 *
 * Copyright (C) 2011 The Chromium OS authors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/tables.h>
#include <asm/arch/sysinfo.h>
#include <video_fb.h>
#include "videomodes.h"

/*
 * The Graphic Device
 */
GraphicDevice ctfb;

static int parse_coreboot_table_fb(GraphicDevice *gdev)
{
	struct cb_framebuffer *fb = lib_sysinfo.framebuffer;

	/* If there is no framebuffer structure, bail out and keep
	 * running on the serial console.
	 */
	if (!fb)
		return 0;

	gdev->winSizeX = fb->x_resolution;
	gdev->winSizeY = fb->y_resolution;

	gdev->plnSizeX = fb->x_resolution;
	gdev->plnSizeY = fb->y_resolution;

	gdev->gdfBytesPP = fb->bits_per_pixel / 8;

	switch (fb->bits_per_pixel) {
	case 24:
		gdev->gdfIndex = GDF_32BIT_X888RGB;
		break;
	case 16:
		gdev->gdfIndex = GDF_16BIT_565RGB;
		break;
	default:
		gdev->gdfIndex = GDF__8BIT_INDEX;
		break;
	}

	gdev->isaBase = CONFIG_SYS_ISA_IO_BASE_ADDRESS;
	gdev->pciBase = (unsigned int)fb->physical_address;

	gdev->frameAdrs = (unsigned int)fb->physical_address;
	gdev->memSize = fb->bytes_per_line * fb->y_resolution;

	gdev->vprBase = (unsigned int)fb->physical_address;
	gdev->cprBase = (unsigned int)fb->physical_address;

	return 1;
}

void *video_hw_init(void)
{
	GraphicDevice *gdev = &ctfb;
	int bits_per_pixel;

	printf("Video: ");

	if (!parse_coreboot_table_fb(gdev)) {
		printf("No video mode configured in coreboot!\n");
		return NULL;
	}

	bits_per_pixel = gdev->gdfBytesPP * 8;

	/* fill in Graphic device struct */
	sprintf(gdev->modeIdent, "%dx%dx%d", gdev->winSizeX, gdev->winSizeY,
		 bits_per_pixel);
	printf("%s\n", gdev->modeIdent);

	memset((void *)gdev->pciBase, 0,
		gdev->winSizeX * gdev->winSizeY * gdev->gdfBytesPP);

	return (void *)gdev;
}
