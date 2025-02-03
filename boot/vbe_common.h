/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Verified Boot for Embedded (VBE) common functions
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VBE_COMMON_H
#define __VBE_COMMON_H

#include <dm/ofnode_decl.h>
#include <linux/bitops.h>
#include <linux/types.h>

struct spl_image_info;
struct udevice;

/*
 * Controls whether we use a full bootmeth driver with VBE in this phase, or
 * just access the information directly.
 *
 * For now VBE-simple uses the full bootmeth, but VBE-abrec does not, to reduce
 * code size
 */
#define USE_BOOTMETH	CONFIG_IS_ENABLED(BOOTMETH_VBE_SIMPLE)

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
 * enum vbe_try_result - result of trying a firmware pick
 *
 * @VBETR_UNKNOWN: Unknown / invalid result
 * @VBETR_TRYING: Firmware pick is being tried
 * @VBETR_OK: Firmware pick is OK and can be used from now on
 * @VBETR_BAD: Firmware pick is bad and should be removed
 */
enum vbe_try_result {
	VBETR_UNKNOWN,
	VBETR_TRYING,
	VBETR_OK,
	VBETR_BAD,
};

/**
 * enum vbe_flags - flags controlling operation
 *
 * @VBEF_TRY_COUNT_MASK: mask for the 'try count' value
 * @VBEF_TRY_B: Try the B slot
 * @VBEF_RECOVERY: Use recovery slot
 */
enum vbe_flags {
	VBEF_TRY_COUNT_MASK	= 0x3,
	VBEF_TRY_B		= BIT(2),
	VBEF_RECOVERY		= BIT(3),

	VBEF_RESULT_SHIFT	= 4,
	VBEF_RESULT_MASK	= 3 << VBEF_RESULT_SHIFT,

	VBEF_PICK_SHIFT		= 6,
	VBEF_PICK_MASK		= 3 << VBEF_PICK_SHIFT,
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

/**
 * vbe_read_version() - Read version-string from a block device
 *
 * Reads the VBE version-string from a device. This function reads a single
 * block from the device, so the string cannot be larger than that. It uses a
 * temporary buffer for the read, then copies in up to @size bytes
 *
 * @blk: Device to read from
 * @offset: Offset to read, in bytes
 * @version: Place to put the string
 * @max_size: Maximum size of @version
 * Return: 0 if OK, -E2BIG if @max_size > block size, -EBADF if the offset is
 * not block-aligned, -EIO if an I/O error occurred
 */
int vbe_read_version(struct udevice *blk, ulong offset, char *version,
		     int max_size);

/**
 * vbe_read_nvdata() - Read non-volatile data from a block device
 *
 * Reads the VBE nvdata from a device. This function reads a single block from
 * the device, so the nvdata cannot be larger than that.
 *
 * @blk: Device to read from
 * @offset: Offset to read, in bytes
 * @size: Number of bytes to read
 * @buf: Buffer to hold the data
 * Return: 0 if OK, -E2BIG if @size > block size, -EBADF if the offset is not
 * block-aligned, -EIO if an I/O error occurred, -EPERM if the header version is
 * incorrect, the header size is invalid or the data fails its CRC check
 */
int vbe_read_nvdata(struct udevice *blk, ulong offset, ulong size, u8 *buf);

/**
 * vbe_read_fit() - Read an image from a FIT
 *
 * This handles most of the VBE logic for reading from a FIT. It reads the FIT
 * metadata, decides which image to load and loads it to a suitable address,
 * ready for jumping to the next phase of VBE.
 *
 * This supports transition from VPL to SPL as well as SPL to U-Boot proper. For
 * now, TPL->VPL is not supported.
 *
 * Both embedded and external data are supported for the FIT
 *
 * @blk: Block device containing FIT
 * @area_offset: Byte offset of the VBE area in @blk containing the FIT
 * @area_size: Size of the VBE area
 * @image: SPL image to fill in with details of the loaded image, or NULL
 * @load_addrp: If non-null, returns the address where the image was loaded
 * @lenp: If non-null, returns the size of the image loaded, in bytes
 * @namep: If non-null, returns the name of the FIT-image node that was loaded
 *	(allocated by this function)
 * Return: 0 if OK, -EINVAL if the area does not contain an FDT (the underlying
 * format for FIT), -E2BIG if the FIT extends past @area_size, -ENOMEM if there
 * was not space to allocate the image-node name, other error if a read error
 * occurred (see blk_read()), or something went wrong with the actually
 * FIT-parsing (see fit_image_load()).
 */
int vbe_read_fit(struct udevice *blk, ulong area_offset, ulong area_size,
		 struct spl_image_info *image, ulong *load_addrp, ulong *lenp,
		 char **namep);

/**
 * vbe_get_node() - Get the node containing the VBE settings
 *
 * Return: VBE node (typically "/bootstd/firmware0")
 */
ofnode vbe_get_node(void);

#endif /* __VBE_ABREC_H */
