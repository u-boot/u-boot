/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 */
#ifndef _FW_LOADER_INTERNAL_H_
#define _FW_LOADER_INTERNAL_H_

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
 * @partoffset: Global offset for BLK partition.
 * @mtdpart: MTD partition for ubi partition.
 * @ubivol: UBI volume-name for ubifsmount.
 */
struct device_plat {
	struct phandle_part phandlepart;
	int partoffset;
	char *mtdpart;
	char *ubivol;

	int (*get_firmware)(struct udevice *dev);
};

/**
 * struct firmware - A place for storing firmware and its attribute data.
 *
 * This holds information about a firmware and its content.
 *
 * @size: Size of a file
 * @data: Buffer for file
 * @priv: Firmware loader private fields
 * @name: Filename
 * @offset: Offset of reading a file
 */
struct firmware {
	size_t size;
	const u8 *data;
	const char *name;
	u32 offset;
};

int generic_fw_loader_probe(struct udevice *dev);
int generic_fw_loader_of_to_plat(struct udevice *dev);

#endif
