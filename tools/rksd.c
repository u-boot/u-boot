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

	size = params->file_size - RK_SPL_HDR_START;
	ret = rkcommon_set_header(buf, size, params);
	if (ret) {
		/* TODO(sjg@chromium.org): This method should return an error */
		printf("Warning: SPL image is too large (size %#x) and will not boot\n",
		       size);
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

/* We pad the file out to a fixed size - this method returns that size */
static int rksd_vrec_header(struct image_tool_params *params,
			    struct image_type_params *tparams)
{
	int pad_size;

	rkcommon_vrec_header(params, tparams);

	pad_size = RK_SPL_HDR_START + rkcommon_get_spl_size(params);
	debug("pad_size %x\n", pad_size);

	return pad_size - params->file_size - tparams->header_size;
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
