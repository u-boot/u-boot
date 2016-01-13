/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MISC_H_
#define _MISC_H_

/*
 * Read the device to buffer, optional.
 *
 * @dev: the device
 * @offset: offset to read the device
 * @buf: pointer to data buffer
 * @size: data size in bytes to read the device
 * @return: 0 if OK, -ve on error
 */
int misc_read(struct udevice *dev, int offset, void *buf, int size);
/*
 * Write buffer to the device, optional.
 *
 * @dev: the device
 * @offset: offset to write the device
 * @buf: pointer to data buffer
 * @size: data size in bytes to write the device
 * @return: 0 if OK, -ve on error
 */
int misc_write(struct udevice *dev, int offset, void *buf, int size);
/*
 * Assert command to the device, optional.
 *
 * @dev: the device
 * @request: command to be sent to the device
 * @buf: pointer to buffer related to the request
 * @return: 0 if OK, -ve on error
 */
int misc_ioctl(struct udevice *dev, unsigned long request, void *buf);

/*
 * struct misc_ops - Driver model Misc operations
 *
 * The uclass interface is implemented by all miscellaneous devices which
 * use driver model.
 */
struct misc_ops {
	/*
	 * Read the device to buffer, optional.
	 *
	 * @dev: the device
	 * @offset: offset to read the device
	 * @buf: pointer to data buffer
	 * @size: data size in bytes to read the device
	 * @return: 0 if OK, -ve on error
	 */
	int (*read)(struct udevice *dev, int offset, void *buf, int size);
	/*
	 * Write buffer to the device, optional.
	 *
	 * @dev: the device
	 * @offset: offset to write the device
	 * @buf: pointer to data buffer
	 * @size: data size in bytes to write the device
	 * @return: 0 if OK, -ve on error
	 */
	int (*write)(struct udevice *dev, int offset, const void *buf,
		     int size);
	/*
	 * Assert command to the device, optional.
	 *
	 * @dev: the device
	 * @request: command to be sent to the device
	 * @buf: pointer to buffer related to the request
	 * @return: 0 if OK, -ve on error
	 */
	int (*ioctl)(struct udevice *dev, unsigned long request, void *buf);
};

#endif	/* _MISC_H_ */
