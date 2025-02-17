/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <linux/i3c/master.h>

/**
 * struct dm_i3c_ops - driver operations for i3c uclass
 *
 * Drivers should support these operations unless otherwise noted. These
 * operations are intended to be used by uclass code, not directly from
 * other code.
 */
struct dm_i3c_ops {
	/**
	 * Transfer messages in I3C mode.
	 *
	 * @see i3c_transfer
	 *
	 * @param dev Pointer to controller device driver instance.
	 * @param target Pointer to target device descriptor.
	 * @param msg Pointer to I3C messages.
	 * @param num_msgs Number of messages to transfer.
	 *
	 * @return @see i3c_transfer
	 */
	int (*i3c_xfers)(struct i3c_dev_desc *dev,
			 struct i3c_priv_xfer *i3c_xfers,
			 int i3c_nxfers);
	int (*read)(struct udevice *dev, u8 dev_number,
		    u8 *buf, int num_bytes);
	int (*write)(struct udevice *dev, u8 dev_number,
		     u8 *buf, int num_bytes);
};

#define i3c_get_ops(dev)	((struct dm_i3c_ops *)(dev)->driver->ops)

/**
 * @brief Do i3c write
 *
 * Uclass general function to start write to i3c target
 *
 * @udevice pointer to i3c controller.
 * @dev_number target device number.
 * @buf target Buffer to write.
 * @num_bytes length of bytes to write.
 *
 * @return 0 for success
 */
int dm_i3c_write(struct udevice *dev, u8 dev_number,
		 u8 *buf, int num_bytes);

/**
 * @brief Do i3c read
 *
 * Uclass general function to start read from i3c target
 *
 * @udevice pointer to i3c controller.
 * @dev_number target device number.
 * @buf target Buffer to read.
 * @num_bytes length of bytes to read.
 *
 * @return 0 for success
 */
int dm_i3c_read(struct udevice *dev, u8 dev_number,
		u8 *buf, int num_bytes);
