/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 *
 * SPDX-License-Identifier:    GPL-2.0
 */
#ifndef _FS_LOADER_H_
#define _FS_LOADER_H_

#include <dm.h>

/**
 * struct firmware - A place for storing firmware and its attribute data.
 *
 * This holds information about a firmware and its content.
 *
 * @size: Size of a file
 * @data: Buffer for file
 * @priv: Firmware loader private fields
 */
struct firmware {
	size_t size;
	const u8 *data;
	void *priv;
};

/**
 * struct phandle_part - A place for storing phandle of node and its partition
 *
 * This holds information about a phandle of the block device, and its
 * partition where the firmware would be loaded from.
 *
 * @phandle: Phandle of storage device node
 * @partition: Partition of block device
 */
struct phandle_part {
	u32 phandle;
	u32 partition;
};

/**
 * struct phandle_part - A place for storing all supported storage devices
 *
 * This holds information about all supported storage devices for driver use.
 *
 * @phandlepart: Attribute data for block device.
 * @mtdpart: MTD partition for ubi partition.
 * @ubivol: UBI volume-name for ubifsmount.
 */
struct device_platdata {
	struct phandle_part phandlepart;
	char *mtdpart;
	char *ubivol;
};

/**
 * release_firmware - Release the resource associated with a firmware image
 * @firmware: Firmware resource to release
 */
void release_firmware(struct firmware *firmware);

/**
 * request_firmware_into_buf - Load firmware into a previously allocated buffer.
 * @plat: Platform data such as storage and partition firmware loading from.
 * @name: Name of firmware file.
 * @buf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 * @firmwarep: Pointer to firmware image.
 *
 * The firmware is loaded directly into the buffer pointed to by @buf and
 * the @firmwarep data member is pointed at @buf.
 *
 * Return: Size of total read, negative value when error.
 */
int request_firmware_into_buf(struct device_platdata *plat,
			      const char *name,
			      void *buf, size_t size, u32 offset,
			      struct firmware **firmwarep);
#endif
