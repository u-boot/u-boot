/*
 * coreboot Framebuffer driver.
 *
 * Copyright (C) 2011 The Chromium OS authors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sysinfo.h>
#include <vbe.h>
#include <video_fb.h>
#include "videomodes.h"

/*
 * The Graphic Device
 */
GraphicDevice ctfb;

static void save_vesa_mode(void)
{
	struct vesa_mode_info *vesa = &mode_info.vesa;
	struct cb_framebuffer *fb = lib_sysinfo.framebuffer;

	vesa->x_resolution = fb->x_resolution;
	vesa->y_resolution = fb->y_resolution;
	vesa->bits_per_pixel = fb->bits_per_pixel;
	vesa->bytes_per_scanline = fb->bytes_per_line;
	vesa->phys_base_ptr = fb->physical_address;
	vesa->red_mask_size = fb->red_mask_size;
	vesa->red_mask_pos = fb->red_mask_pos;
	vesa->green_mask_size = fb->green_mask_size;
	vesa->green_mask_pos = fb->green_mask_pos;
	vesa->blue_mask_size = fb->blue_mask_size;
	vesa->blue_mask_pos = fb->blue_mask_pos;
	vesa->reserved_mask_size = fb->reserved_mask_size;
	vesa->reserved_mask_pos = fb->reserved_mask_pos;
}

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

	/* Initialize vesa_mode_info structure */
	save_vesa_mode();

	return (void *)gdev;
}
