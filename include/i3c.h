/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <linux/i3c/master.h>

/**
 * struct dm_i3c_ops - Driver operations for the I3C uclass
 *
 * This structure defines the set of operations that a driver must implement
 * for interacting with an I3C controller in U-Boot.
 *
 */
struct dm_i3c_ops {
/**
 * @i3c_xfers: Transfer messages in I3C
 *
 * @dev: I3C controller device instance.
 * @xfers: List of I3C private SDR transfer messages.
 * @nxfers: The number of messages to transfer.
 *
 * Return: 0 on success, negative error code on failure.
 */
	int (*i3c_xfers)(struct i3c_dev_desc *dev,
			 struct i3c_priv_xfer *xfers,
			 u32 nxfers);

/**
 * @read: Perform I3C read transaction.
 *
 * @dev: Chip to read from
 * @dev_number: The target device number from the driver model.
 * @buf: Place to put data
 * @num_bytes: Number of bytes to read.
 *
 * Return: 0 on success, negative error code on failure.
 */
	int (*read)(struct udevice *dev, u32 dev_number,
		    u8 *buf, u32 num_bytes);

/**
 * @write: Perform I3C write transaction.
 *
 * @dev: Chip to write to
 * @dev_number: The target device number from the driver model.
 * @buf: Buffer containing data to write
 * @num_bytes: Number of bytes to write.
 *
 * Return: 0 on success, negative error code on failure.
 */
	int (*write)(struct udevice *dev, u32 dev_number,
		     u8 *buf, u32 num_bytes);
};

/**
 * i3c_get_ops - Retrieve the I3C operation functions for a device
 * @dev: The I3C controller device.
 *
 * This macro returns the set of operation functions (`dm_i3c_ops`) implemented
 * by the driver associated with the specified device. These operations define
 * how the driver performs I3C communication tasks such as reading, writing,
 * and message transfers.
 *
 * Return: The I3C operation structure for the device.
 */
#define i3c_get_ops(dev)	((struct dm_i3c_ops *)(dev)->driver->ops)

/**
 * dm_i3c_write - Perform I3C write transaction
 *
 * @dev: Chip to write to
 * @dev_number: The target device number from the driver model.
 * @buf: Buffer containing data to write
 * @num_bytes: Number of bytes to write.
 *
 * Return: 0 on success, negative error code on failure.
 */
int dm_i3c_write(struct udevice *dev, u32 dev_number,
		 u8 *buf, u32 num_bytes);

/**
 * dm_i3c_read - Perform I3C read transaction
 *
 * @dev: Chip to read from
 * @dev_number: The target device number from the driver model.
 * @buf: Place to put data
 * @num_bytes: Number of bytes to read.
 *
 * Return: 0 on success, negative error code on failure.
 */
int dm_i3c_read(struct udevice *dev, u32 dev_number,
		u8 *buf, u32 num_bytes);
