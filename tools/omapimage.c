/*
 * (C) Copyright 2010
 * Linaro LTD, www.linaro.org
 * Author: John Rigby <john.rigby@linaro.org>
 * Based on TI's signGP.c
 *
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"
#include <image.h>
#include "omapimage.h"

/* Header size is CH header rounded up to 512 bytes plus GP header */
#define OMAP_CH_HDR_SIZE 512
#define OMAP_GP_HDR_SIZE (sizeof(struct gp_header))
#define OMAP_FILE_HDR_SIZE (OMAP_CH_HDR_SIZE+OMAP_GP_HDR_SIZE)

static int do_swap32 = 0;

static uint32_t omapimage_swap32(uint32_t data)
{
	uint32_t result = 0;
	result  = (data & 0xFF000000) >> 24;
	result |= (data & 0x00FF0000) >> 8;
	result |= (data & 0x0000FF00) << 8;
	result |= (data & 0x000000FF) << 24;
	return result;
}

static uint8_t omapimage_header[OMAP_FILE_HDR_SIZE];

static int omapimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_OMAPIMAGE)
		return EXIT_SUCCESS;
	else {
		return EXIT_FAILURE;
	}
}

/*
 * Only the simplest image type is currently supported:
 * TOC pointing to CHSETTINGS
 * TOC terminator
 * CHSETTINGS
 *
 * padding to OMAP_CH_HDR_SIZE bytes
 *
 * gp header
 *   size
 *   load_addr
 */
static int valid_gph_size(uint32_t size)
{
	return size;
}

static int valid_gph_load_addr(uint32_t load_addr)
{
	return load_addr;
}

static int omapimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct ch_toc *toc = (struct ch_toc *)ptr;
	struct gp_header *gph = (struct gp_header *)(ptr+OMAP_CH_HDR_SIZE);
	uint32_t offset, size, gph_size, gph_load_addr;

	while (toc->section_offset != 0xffffffff
			&& toc->section_size != 0xffffffff) {
		if (do_swap32) {
			offset = omapimage_swap32(toc->section_offset);
			size = omapimage_swap32(toc->section_size);
		} else {
			offset = toc->section_offset;
			size = toc->section_size;
		}
		if (!offset || !size)
			return -1;
		if (offset >= OMAP_CH_HDR_SIZE ||
		    offset+size >= OMAP_CH_HDR_SIZE)
			return -1;
		toc++;
	}

	if (do_swap32) {
		gph_size = omapimage_swap32(gph->size);
		gph_load_addr = omapimage_swap32(gph->load_addr);
	} else {
		gph_size = gph->size;
		gph_load_addr = gph->load_addr;
	}

	if (!valid_gph_size(gph_size))
		return -1;
	if (!valid_gph_load_addr(gph_load_addr))
		return -1;

	return 0;
}

static void omapimage_print_section(struct ch_settings *chs)
{
	const char *section_name;

	if (chs->section_key)
		section_name = "CHSETTINGS";
	else
		section_name = "UNKNOWNKEY";

	printf("%s (%x) "
		"valid:%x "
		"version:%x "
		"reserved:%x "
		"flags:%x\n",
		section_name,
		chs->section_key,
		chs->valid,
		chs->version,
		chs->reserved,
		chs->flags);
}

static void omapimage_print_header(const void *ptr)
{
	const struct ch_toc *toc = (struct ch_toc *)ptr;
	const struct gp_header *gph =
			(struct gp_header *)(ptr+OMAP_CH_HDR_SIZE);
	uint32_t offset, size, gph_size, gph_load_addr;

	while (toc->section_offset != 0xffffffff
			&& toc->section_size != 0xffffffff) {
		if (do_swap32) {
			offset = omapimage_swap32(toc->section_offset);
			size = omapimage_swap32(toc->section_size);
		} else {
			offset = toc->section_offset;
			size = toc->section_size;
		}

		if (offset >= OMAP_CH_HDR_SIZE ||
		    offset+size >= OMAP_CH_HDR_SIZE)
			exit(EXIT_FAILURE);

		printf("Section %s offset %x length %x\n",
			toc->section_name,
			toc->section_offset,
			toc->section_size);

		omapimage_print_section((struct ch_settings *)(ptr+offset));
		toc++;
	}

	if (do_swap32) {
		gph_size = omapimage_swap32(gph->size);
		gph_load_addr = omapimage_swap32(gph->load_addr);
	} else {
		gph_size = gph->size;
		gph_load_addr = gph->load_addr;
	}

	if (!valid_gph_size(gph_size)) {
		fprintf(stderr, "Error: invalid image size %x\n", gph_size);
		exit(EXIT_FAILURE);
	}

	if (!valid_gph_load_addr(gph_load_addr)) {
		fprintf(stderr, "Error: invalid image load address %x\n",
				gph_load_addr);
		exit(EXIT_FAILURE);
	}

	printf("GP Header: Size %x LoadAddr %x\n", gph_size, gph_load_addr);
}

static int toc_offset(void *hdr, void *member)
{
	return member - hdr;
}

static void omapimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct ch_toc *toc = (struct ch_toc *)ptr;
	struct ch_settings *chs = (struct ch_settings *)
					(ptr + 2 * sizeof(*toc));
	struct gp_header *gph = (struct gp_header *)(ptr + OMAP_CH_HDR_SIZE);

	toc->section_offset = toc_offset(ptr, chs);
	toc->section_size = sizeof(struct ch_settings);
	strcpy((char *)toc->section_name, "CHSETTINGS");

	chs->section_key = KEY_CHSETTINGS;
	chs->valid = 0;
	chs->version = 1;
	chs->reserved = 0;
	chs->flags = 0;

	toc++;
	memset(toc, 0xff, sizeof(*toc));

	gph->size = sbuf->st_size - OMAP_FILE_HDR_SIZE;
	gph->load_addr = params->addr;

	if (strncmp(params->imagename, "byteswap", 8) == 0) {
		do_swap32 = 1;
		int swapped = 0;
		uint32_t *data = (uint32_t *)ptr;

		while (swapped <= (sbuf->st_size / sizeof(uint32_t))) {
			*data = omapimage_swap32(*data);
			swapped++;
			data++;
		}
	}
}

int omapimage_check_params(struct image_tool_params *params)
{
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag));
}

/*
 * omapimage parameters
 */
static struct image_type_params omapimage_params = {
	.name		= "TI OMAP CH/GP Boot Image support",
	.header_size	= OMAP_FILE_HDR_SIZE,
	.hdr		= (void *)&omapimage_header,
	.check_image_type = omapimage_check_image_types,
	.verify_header	= omapimage_verify_header,
	.print_header	= omapimage_print_header,
	.set_header	= omapimage_set_header,
	.check_params	= omapimage_check_params,
};

void init_omap_image_type(void)
{
	register_image_type(&omapimage_params);
}
