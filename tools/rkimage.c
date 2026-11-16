// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * rkimage.c combines the TPL/SPL binaries into a single image, sets the
 * magic value expected by the SoC in the first 4 bytes and RC4-encodes
 * the result if needed. The output is typically used as payload
 * for the Mask ROM, uploaded via USB OTG or stored on a boot medium.
 */

#include "imagetool.h"
#include <image.h>
#include "rkcommon.h"

static uint32_t header;

static void rkimage_set_header(void *buf, struct stat *sbuf, int ifd,
			       struct image_tool_params *params)
{
	memcpy(buf, rkcommon_get_spl_hdr(params), RK_SPL_HDR_SIZE);

	if (rkcommon_need_rc4_spl(params))
		rkcommon_rc4_encode_spl(buf, 0, params->file_size);
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
	0,
	&header,
	rkcommon_check_params,
	NULL,
	NULL,
	rkimage_set_header,
	NULL,
	rkimage_check_image_type,
	NULL,
	NULL
);
