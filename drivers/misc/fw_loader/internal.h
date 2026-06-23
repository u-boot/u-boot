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
 * struct fw_loader_ops - driver operations for Firmware Loader uclass
 *
 * Drivers MUST support these operation. These operations are intended
 * to be used by uclass code, not directly from other code.
 */
struct fw_loader_ops {
	/**
	 * get_firmware() - get firmware from Firmware Loader driver
	 *
	 * @dev:	Firmware Loader device to read firmware from
	 */
	int (*get_firmware)(struct udevice *dev);

	/**
	 * get_size() - get firmware size from Firmware Loader driver
	 *
	 * @dev:	Firmware Loader device to get firmware size from
	 */
	int (*get_size)(struct udevice *dev);
};

#define fw_loader_get_ops(dev)	((struct fw_loader_ops *)(dev)->driver->ops)

/**
 * struct device_plat - A place for storing all supported storage devices
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
	u32 partoffset;
	char *mtdpart;
	char *ubivol;
};

/**
 * struct firmware - A place for storing firmware and its attribute data.
 *
 * This holds information about a firmware and its content.
 *
 * @size: Size of a file
 * @data: Buffer for file
 * @name: Filename
 * @offset: Offset of reading a file
 */
struct firmware {
	size_t size;
	const u8 *data;
	const char *name;
	u32 offset;
};

int generic_fw_loader_ubi_select(char *mtdpart);

#endif
