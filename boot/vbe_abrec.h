/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Verified Boot for Embedded (VBE) vbe-abrec common file
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VBE_ABREC_H
#define __VBE_ABREC_H

#include <vbe.h>
#include <dm/ofnode_decl.h>

#include "vbe_common.h"

struct bootflow;
struct udevice;

/**
 * struct abrec_priv - information read from the device tree
 *
 * @area_start: Start offset of the VBE area in the device, in bytes
 * @area_size: Total size of the VBE area
 * @skip_offset: Size of an initial part of the device to skip, when using
 *	area_start. This is effectively added to area_start to calculate the
 *	actual start position on the device
 * @state_offset: Offset from area_start of the VBE state, in bytes
 * @state_size: Size of the state information
 * @version_offset: Offset from from area_start of the VBE version info
 * @version_size: Size of the version info
 * @storage: Storage device to use, in the form <uclass><devnum>, e.g. "mmc1"
 */
struct abrec_priv {
	u32 area_start;
	u32 area_size;
	u32 skip_offset;
	u32 state_offset;
	u32 state_size;
	u32 version_offset;
	u32 version_size;
	const char *storage;
};

/** struct abrec_state - state information read from media
 *
 * The state on the media is converted into this more code-friendly structure.
 *
 * @fw_version: Firmware version string
 * @fw_vernum: Firmware version number
 * @try_count: Number of times the B firmware has been tried
 * @try_b: true to try B firmware on the next boot
 * @recovery: true to enter recovery firmware on the next boot
 * @try_result: Result of trying to boot with the last firmware
 * @pick: Firmware which was chosen in this boot
 */
struct abrec_state {
	char fw_version[MAX_VERSION_LEN];
	u32 fw_vernum;
	u8 try_count;
	bool try_b;
	bool recovery;
	enum vbe_try_result try_result;
	enum vbe_pick_t pick;
};

/**
 * abrec_read_fw_bootflow() - Read a bootflow for firmware
 *
 * Locates and loads the firmware image (FIT) needed for the next phase. The FIT
 * should ideally use external data, to reduce the amount of it that needs to be
 * read.
 *
 * @bdev: bootdev device containing the firmwre
 * @bflow: Place to put the created bootflow, on success
 * @return 0 if OK, -ve on error
 */
int abrec_read_bootflow_fw(struct udevice *dev, struct bootflow *bflow);

/**
 * vbe_simple_read_state() - Read the VBE simple state information
 *
 * @dev: VBE bootmeth
 * @state: Place to put the state
 * @return 0 if OK, -ve on error
 */
int abrec_read_state(struct udevice *dev, struct abrec_state *state);

/**
 * abrec_read_nvdata() - Read non-volatile data from a block device
 *
 * Reads the ABrec VBE nvdata from a device. This function reads a single block
 * from the device, so the nvdata cannot be larger than that.
 *
 * @blk: Device to read from
 * @offset: Offset to read, in bytes
 * @size: Number of bytes to read
 * @buf: Buffer to hold the data
 * Return: 0 if OK, -E2BIG if @size > block size, -EBADF if the offset is not
 * block-aligned, -EIO if an I/O error occurred, -EPERM if the header version is
 * incorrect, the header size is invalid or the data fails its CRC check
 */
int abrec_read_nvdata(struct abrec_priv *priv, struct udevice *blk,
		      struct abrec_state *state);

/**
 * abrec_read_priv() - Read info from the devicetree
 *
 * @node: Node to read from
 * @priv: Information to fill in
 * Return 0 if OK, -EINVAL if something is wrong with the devicetree node
 */
int abrec_read_priv(ofnode node, struct abrec_priv *priv);

#endif /* __VBE_ABREC_H */
