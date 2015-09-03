/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * See README.rockchip for details of the rkspi format
 */

#include "imagetool.h"
#include <image.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

enum {
	RKSPI_SPL_HDR_START	= RK_CODE1_OFFSET * RK_BLK_SIZE,
	RKSPI_SPL_START		= RKSPI_SPL_HDR_START + 4,
	RKSPI_HEADER_LEN	= RKSPI_SPL_START,
	RKSPI_SECT_LEN		= RK_BLK_SIZE * 4,
};

static char dummy_hdr[RKSPI_HEADER_LEN];

static int rkspi_check_params(struct image_tool_params *params)
{
	return 0;
}

static int rkspi_verify_header(unsigned char *buf, int size,
			       struct image_tool_params *params)
{
	return 0;
}

static void rkspi_print_header(const void *buf)
{
}

static void rkspi_set_header(void *buf, struct stat *sbuf, int ifd,
			     struct image_tool_params *params)
{
	int sector;
	unsigned int size;
	int ret;

	size = params->orig_file_size;
	ret = rkcommon_set_header(buf, size);
	debug("size %x\n", size);
	if (ret) {
		/* TODO(sjg@chromium.org): This method should return an error */
		printf("Warning: SPL image is too large (size %#x) and will not boot\n",
		       size);
	}

	memcpy(buf + RKSPI_SPL_HDR_START, "RK32", 4);

	/*
	 * Spread the image out so we only use the first 2KB of each 4KB
	 * region. This is a feature of the SPI format required by the Rockchip
	 * boot ROM. Its rationale is unknown.
	 */
	for (sector = size / RKSPI_SECT_LEN - 1; sector >= 0; sector--) {
		printf("sector %u\n", sector);
		memmove(buf + sector * RKSPI_SECT_LEN * 2,
			buf + sector * RKSPI_SECT_LEN,
			RKSPI_SECT_LEN);
		memset(buf + sector * RKSPI_SECT_LEN * 2 + RKSPI_SECT_LEN,
		       '\0', RKSPI_SECT_LEN);
	}
}

static int rkspi_extract_subimage(void *buf, struct image_tool_params *params)
{
	return 0;
}

static int rkspi_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_RKSPI)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/* We pad the file out to a fixed size - this method returns that size */
static int rkspi_vrec_header(struct image_tool_params *params,
			     struct image_type_params *tparams)
{
	int pad_size;

	pad_size = (RK_MAX_CODE1_SIZE + 0x7ff) / 0x800 * 0x800;
	params->orig_file_size = pad_size;

	/* We will double the image size due to the SPI format */
	pad_size *= 2;
	pad_size += RKSPI_SPL_HDR_START;
	debug("pad_size %x\n", pad_size);

	return pad_size - params->file_size;
}

/*
 * rk_spi parameters
 */
U_BOOT_IMAGE_TYPE(
	rkspi,
	"Rockchip SPI Boot Image support",
	RKSPI_HEADER_LEN,
	dummy_hdr,
	rkspi_check_params,
	rkspi_verify_header,
	rkspi_print_header,
	rkspi_set_header,
	rkspi_extract_subimage,
	rkspi_check_image_type,
	NULL,
	rkspi_vrec_header
);
