/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Verified Boot for Embedded (VBE) common functions
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VBE_COMMON_H
#define __VBE_COMMON_H

#include <linux/types.h>

struct udevice;

enum {
	MAX_VERSION_LEN		= 256,

	NVD_HDR_VER_SHIFT	= 0,
	NVD_HDR_VER_MASK	= 0xf,
	NVD_HDR_SIZE_SHIFT	= 4,
	NVD_HDR_SIZE_MASK	= 0xf << NVD_HDR_SIZE_SHIFT,

	/* Firmware key-version is in the top 16 bits of fw_ver */
	FWVER_KEY_SHIFT		= 16,
	FWVER_FW_MASK		= 0xffff,

	NVD_HDR_VER_CUR		= 1,	/* current version */
};

/**
 * struct vbe_nvdata - basic storage format for non-volatile data
 *
 * This is used for all VBE methods
 *
 * @crc8: crc8 for the entire record except @crc8 field itself
 * @hdr: header size and version (NVD_HDR_...)
 * @spare1: unused, must be 0
 * @fw_vernum: version and key version (FWVER_...)
 * @flags: Flags controlling operation (enum vbe_flags)
 */
struct vbe_nvdata {
	u8 crc8;
	u8 hdr;
	u16 spare1;
	u32 fw_vernum;
	u32 flags;
	u8 spare2[0x34];
};

/**
 * vbe_get_blk() - Obtain the block device to use for VBE
 *
 * Decodes the string to produce a block device
 *
 * @storage: String indicating the device to use, e.g. "mmc1"
 * @blkp: Returns associated block device, on success
 * Return 0 if OK, -ENODEV if @storage does not end with a number, -E2BIG if
 * the device name is more than 15 characters, -ENXIO if the block device could
 * not be found
 */
int vbe_get_blk(const char *storage, struct udevice **blkp);

#endif /* __VBE_ABREC_H */
