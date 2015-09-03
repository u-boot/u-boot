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
 * @code1_offset:	Offset in blocks of the SPL code from this header
 *			block. E.g. 4 means 2KB after the start of this header.
 * Other fields are not used by U-Boot
 */
struct header0_info {
	uint32_t signature;
	uint8_t reserved[4];
	uint32_t disable_rc4;
	uint16_t code1_offset;
	uint16_t code2_offset;
	uint8_t reserved1[490];
	uint16_t usflashdatasize;
	uint16_t ucflashbootsize;
	uint8_t reserved2[2];
};

static unsigned char rc4_key[16] = {
	124, 78, 3, 4, 85, 5, 9, 7,
	45, 44, 123, 56, 23, 13, 23, 17
};

int rkcommon_set_header(void *buf, uint file_size)
{
	struct header0_info *hdr;

	if (file_size > RK_MAX_CODE1_SIZE)
		return -ENOSPC;

	memset(buf,  '\0', RK_CODE1_OFFSET * RK_BLK_SIZE);
	hdr = (struct header0_info *)buf;
	hdr->signature = RK_SIGNATURE;
	hdr->disable_rc4 = 1;
	hdr->code1_offset = RK_CODE1_OFFSET;
	hdr->code2_offset = 8;

	hdr->usflashdatasize = (file_size + RK_BLK_SIZE - 1) / RK_BLK_SIZE;
	hdr->usflashdatasize = (hdr->usflashdatasize + 3) & ~3;
	hdr->ucflashbootsize = hdr->usflashdatasize;

	debug("size=%x, %x\n", params->file_size, hdr->usflashdatasize);

	rc4_encode(buf, RK_BLK_SIZE, rc4_key);

	return 0;
}
