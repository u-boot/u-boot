/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Helper functions for Rockchip images
 */

#include "imagetool.h"
#include <image.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

enum {
	RK_SIGNATURE		= 0x0ff0aa55,
};

/**
 * struct header0_info - header block for boot ROM
 *
 * This is stored at SD card block 64 (where each block is 512 bytes, or at
 * the start of SPI flash. It is encoded with RC4.
 *
 * @signature:		Signature (must be RKSD_SIGNATURE)
 * @disable_rc4:	0 to use rc4 for boot image,  1 to use plain binary
 * @init_offset:	Offset in blocks of the SPL code from this header
 *			block. E.g. 4 means 2KB after the start of this header.
 * Other fields are not used by U-Boot
 */
struct header0_info {
	uint32_t signature;
	uint8_t reserved[4];
	uint32_t disable_rc4;
	uint16_t init_offset;
	uint8_t reserved1[492];
	uint16_t init_size;
	uint16_t init_boot_size;
	uint8_t reserved2[2];
};

/**
 * struct header1_info
 */
struct header1_info {
	uint32_t magic;
};

/**
 * struct spl_info - spl info for each chip
 *
 * @imagename:		Image name(passed by "mkimage -n")
 * @spl_hdr:		Boot ROM requires a 4-bytes spl header
 * @spl_size:		Spl size(include extra 4-bytes spl header)
 * @spl_rc4:		RC4 encode the SPL binary (same key as header)
 * @spl_boot0:          A new-style (ARM_SOC_BOOT0_HOOK) image that should
 *                      have the boot magic (e.g. 'RK33') written to its first
 *                      word.
 */

struct spl_info {
	const char *imagename;
	const char *spl_hdr;
	const uint32_t spl_size;
	const bool spl_rc4;
	const bool spl_boot0;
};

static struct spl_info spl_infos[] = {
	{ "rk3036", "RK30", 0x1000, false, false },
	{ "rk3188", "RK31", 0x8000 - 0x800, true, false },
	{ "rk3288", "RK32", 0x8000, false, false },
	{ "rk3399", "RK33", 0x20000, false, true },
};

static unsigned char rc4_key[16] = {
	124, 78, 3, 4, 85, 5, 9, 7,
	45, 44, 123, 56, 23, 13, 23, 17
};

static struct spl_info *rkcommon_get_spl_info(char *imagename)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(spl_infos); i++)
		if (!strncmp(imagename, spl_infos[i].imagename, 6))
			return spl_infos + i;

	return NULL;
}

int rkcommon_check_params(struct image_tool_params *params)
{
	int i;

	if (rkcommon_get_spl_info(params->imagename) != NULL)
		return 0;

	fprintf(stderr, "ERROR: imagename (%s) is not supported!\n",
		strlen(params->imagename) > 0 ? params->imagename : "NULL");

	fprintf(stderr, "Available imagename:");
	for (i = 0; i < ARRAY_SIZE(spl_infos); i++)
		fprintf(stderr, "\t%s", spl_infos[i].imagename);
	fprintf(stderr, "\n");

	return -1;
}

const char *rkcommon_get_spl_hdr(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_hdr;
}


int rkcommon_get_spl_size(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_size;
}

bool rkcommon_need_rc4_spl(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_rc4;
}

bool rkcommon_spl_is_boot0(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_boot0;
}

static void rkcommon_set_header0(void *buf, uint file_size,
				 struct image_tool_params *params)
{
	struct header0_info *hdr = buf;

	memset(buf, '\0', RK_INIT_OFFSET * RK_BLK_SIZE);
	hdr->signature = RK_SIGNATURE;
	hdr->disable_rc4 = !rkcommon_need_rc4_spl(params);
	hdr->init_offset = RK_INIT_OFFSET;

	hdr->init_size = (file_size + RK_BLK_SIZE - 1) / RK_BLK_SIZE;
	hdr->init_size = (hdr->init_size + 3) & ~3;
	hdr->init_boot_size = hdr->init_size + RK_MAX_BOOT_SIZE / RK_BLK_SIZE;

	rc4_encode(buf, RK_BLK_SIZE, rc4_key);
}

int rkcommon_set_header(void *buf, uint file_size,
			struct image_tool_params *params)
{
	struct header1_info *hdr = buf + RK_SPL_HDR_START;

	if (file_size > rkcommon_get_spl_size(params))
		return -ENOSPC;

	rkcommon_set_header0(buf, file_size, params);

	/* Set up the SPL name and add the AArch64 'nop' padding, if needed */
	memcpy(&hdr->magic, rkcommon_get_spl_hdr(params), RK_SPL_HDR_SIZE);

	if (rkcommon_need_rc4_spl(params))
		rkcommon_rc4_encode_spl(buf, RK_SPL_HDR_START,
					params->file_size - RK_SPL_HDR_START);

	return 0;
}

void rkcommon_rc4_encode_spl(void *buf, unsigned int offset, unsigned int size)
{
	unsigned int remaining = size;

	while (remaining > 0) {
		int step = (remaining > RK_BLK_SIZE) ? RK_BLK_SIZE : remaining;

		rc4_encode(buf + offset, step, rc4_key);
		offset += RK_BLK_SIZE;
		remaining -= step;
	}
}

void rkcommon_vrec_header(struct image_tool_params *params,
			  struct image_type_params *tparams)
{
	/*
	 * The SPL image looks as follows:
	 *
	 * 0x0    header0 (see rkcommon.c)
	 * 0x800  spl_name ('RK30', ..., 'RK33')
	 * 0x804  first instruction to be executed
	 *        (image start for AArch32, 'nop' for AArch64))
	 * 0x808  second instruction to be executed
	 *        (image start for AArch64)
	 *
	 * For AArch64 (ARMv8) payloads, we receive an input file that
	 * needs to start on an 8-byte boundary (natural alignment), so
	 * we need to put a NOP at 0x804.
	 *
	 * Depending on this, the header is either 0x804 or 0x808 bytes
	 * in length.
	 */
	if (rkcommon_spl_is_boot0(params))
		tparams->header_size = RK_SPL_HDR_START;
	else
		tparams->header_size = RK_SPL_HDR_START + 4;

	/* Allocate, clear and install the header */
	tparams->hdr = malloc(tparams->header_size);
	memset(tparams->hdr, 0, tparams->header_size);
}
