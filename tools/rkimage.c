/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * See README.rockchip for details of the rkimage format
 */

#include "imagetool.h"
#include <image.h>

static uint32_t header;

static int rkimage_check_params(struct image_tool_params *params)
{
	return 0;
}

static int rkimage_verify_header(unsigned char *buf, int size,
				 struct image_tool_params *params)
{
	return 0;
}

static void rkimage_print_header(const void *buf)
{
}

static void rkimage_set_header(void *buf, struct stat *sbuf, int ifd,
			       struct image_tool_params *params)
{
	memcpy(buf, "RK32", 4);
}

static int rkimage_extract_subimage(void *buf, struct image_tool_params *params)
{
	return 0;
}

static int rkimage_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/*
 * rk_image parameters
 */
U_BOOT_IMAGE_TYPE(
	rkimage,
	"Rockchip Boot Image support",
	4,
	&header,
	rkimage_check_params,
	rkimage_verify_header,
	rkimage_print_header,
	rkimage_set_header,
	rkimage_extract_subimage,
	rkimage_check_image_type,
	NULL,
	NULL
);
