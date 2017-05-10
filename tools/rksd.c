/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * See README.rockchip for details of the rksd format
 */

#include "imagetool.h"
#include <image.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

static int rksd_verify_header(unsigned char *buf,  int size,
				 struct image_tool_params *params)
{
	return 0;
}

static void rksd_print_header(const void *buf)
{
}

static void rksd_set_header(void *buf,  struct stat *sbuf,  int ifd,
			       struct image_tool_params *params)
{
	unsigned int size;
	int ret;

	printf("params->file_size %d\n", params->file_size);
	printf("params->orig_file_size %d\n", params->orig_file_size);

	/*
	 * We need to calculate this using 'RK_SPL_HDR_START' and not using
	 * 'tparams->header_size', as the additional byte inserted when
	 * 'is_boot0' is true counts towards the payload.
	 */
	size = params->file_size - RK_SPL_HDR_START;
	ret = rkcommon_set_header(buf, size, params);
	if (ret) {
		/* TODO(sjg@chromium.org): This method should return an error */
		printf("Warning: SPL image is too large (size %#x) and will "
		       "not boot\n", size);
	}
}

static int rksd_extract_subimage(void *buf,  struct image_tool_params *params)
{
	return 0;
}

static int rksd_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKSD)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int rksd_vrec_header(struct image_tool_params *params,
			    struct image_type_params *tparams)
{
	/*
	 * Pad to the RK_BLK_SIZE (512 bytes) to be consistent with init_size
	 * being encoded in RK_BLK_SIZE units in header0 (see rkcommon.c).
	 */
	return rkcommon_vrec_header(params, tparams, RK_BLK_SIZE);
}

/*
 * rk_sd parameters
 */
U_BOOT_IMAGE_TYPE(
	rksd,
	"Rockchip SD Boot Image support",
	0,
	NULL,
	rkcommon_check_params,
	rksd_verify_header,
	rksd_print_header,
	rksd_set_header,
	rksd_extract_subimage,
	rksd_check_image_type,
	NULL,
	rksd_vrec_header
);
