/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
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
 * Send a message to the device and wait for a response.
 *
 * The caller provides the message type/ID and payload to be sent.
 * The callee constructs any message header required, transmits it to the
 * target, waits for a response, checks any error code in the response,
 * strips any message header from the response, and returns the error code
 * (or a parsed version of it) and the response message payload.
 *
 * @dev: the device.
 * @msgid: the message ID/number to send.
 * tx_msg: the request/transmit message payload.
 * tx_size: the size of the buffer pointed at by tx_msg.
 * rx_msg: the buffer to receive the response message payload. May be NULL if
 *         the caller only cares about the error code.
 * rx_size: the size of the buffer pointed at by rx_msg.
 * @return the response message size if OK, -ve on error
 */
int misc_call(struct udevice *dev, int msgid, void *tx_msg, int tx_size,
	      void *rx_msg, int rx_size);

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
	/*
	 * Send a message to the device and wait for a response.
	 *
	 * @dev: the device
	 * @msgid: the message ID/number to send
	 * tx_msg: the request/transmit message payload
	 * tx_size: the size of the buffer pointed at by tx_msg
	 * rx_msg: the buffer to receive the response message payload. May be
	 *         NULL if the caller only cares about the error code.
	 * rx_size: the size of the buffer pointed at by rx_msg
	 * @return the response message size if OK, -ve on error
	 */
	int (*call)(struct udevice *dev, int msgid, void *tx_msg, int tx_size,
		    void *rx_msg, int rx_size);
};

#endif	/* _MISC_H_ */
