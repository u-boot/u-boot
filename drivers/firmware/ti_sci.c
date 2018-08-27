// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments System Control Interface Protocol Driver
 * Based on drivers/firmware/ti_sci.c from Linux.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mailbox.h>
#include <dm/device.h>
#include <linux/err.h>
#include <linux/soc/ti/k3-sec-proxy.h>
#include <linux/soc/ti/ti_sci_protocol.h>

#include "ti_sci.h"

/* List of all TI SCI devices active in system */
static LIST_HEAD(ti_sci_list);

/**
 * struct ti_sci_xfer - Structure representing a message flow
 * @tx_message:	Transmit message
 * @rx_len:	Receive message length
 */
struct ti_sci_xfer {
	struct k3_sec_proxy_msg tx_message;
	u8 rx_len;
};

/**
 * struct ti_sci_desc - Description of SoC integration
 * @host_id:		Host identifier representing the compute entity
 * @max_rx_timeout_us:	Timeout for communication with SoC (in Microseconds)
 * @max_msg_size:	Maximum size of data per message that can be handled.
 */
struct ti_sci_desc {
	u8 host_id;
	int max_rx_timeout_us;
	int max_msg_size;
};

/**
 * struct ti_sci_info - Structure representing a TI SCI instance
 * @dev:	Device pointer
 * @desc:	SoC description for this instance
 * @handle:	Instance of TI SCI handle to send to clients.
 * @chan_tx:	Transmit mailbox channel
 * @chan_rx:	Receive mailbox channel
 * @xfer:	xfer info
 * @list:	list head
 * @is_secure:	Determines if the communication is through secure threads.
 * @host_id:	Host identifier representing the compute entity
 * @seq:	Seq id used for verification for tx and rx message.
 */
struct ti_sci_info {
	struct udevice *dev;
	const struct ti_sci_desc *desc;
	struct ti_sci_handle handle;
	struct mbox_chan chan_tx;
	struct mbox_chan chan_rx;
	struct mbox_chan chan_notify;
	struct ti_sci_xfer xfer;
	struct list_head list;
	bool is_secure;
	u8 host_id;
	u8 seq;
};

#define handle_to_ti_sci_info(h) container_of(h, struct ti_sci_info, handle)

/**
 * ti_sci_setup_one_xfer() - Setup one message type
 * @info:	Pointer to SCI entity information
 * @msg_type:	Message type
 * @msg_flags:	Flag to set for the message
 * @buf:	Buffer to be send to mailbox channel
 * @tx_message_size: transmit message size
 * @rx_message_size: receive message size
 *
 * Helper function which is used by various command functions that are
 * exposed to clients of this driver for allocating a message traffic event.
 *
 * Return: Corresponding ti_sci_xfer pointer if all went fine,
 *	   else appropriate error pointer.
 */
static struct ti_sci_xfer *ti_sci_setup_one_xfer(struct ti_sci_info *info,
						 u16 msg_type, u32 msg_flags,
						 u32 *buf,
						 size_t tx_message_size,
						 size_t rx_message_size)
{
	struct ti_sci_xfer *xfer = &info->xfer;
	struct ti_sci_msg_hdr *hdr;

	/* Ensure we have sane transfer sizes */
	if (rx_message_size > info->desc->max_msg_size ||
	    tx_message_size > info->desc->max_msg_size ||
	    rx_message_size < sizeof(*hdr) || tx_message_size < sizeof(*hdr))
		return ERR_PTR(-ERANGE);

	info->seq = ~info->seq;
	xfer->tx_message.buf = buf;
	xfer->tx_message.len = tx_message_size;
	xfer->rx_len = (u8)rx_message_size;

	hdr = (struct ti_sci_msg_hdr *)buf;
	hdr->seq = info->seq;
	hdr->type = msg_type;
	hdr->host = info->host_id;
	hdr->flags = msg_flags;

	return xfer;
}

/**
 * ti_sci_get_response() - Receive response from mailbox channel
 * @info:	Pointer to SCI entity information
 * @xfer:	Transfer to initiate and wait for response
 * @chan:	Channel to receive the response
 *
 * Return: -ETIMEDOUT in case of no response, if transmit error,
 *	   return corresponding error, else if all goes well,
 *	   return 0.
 */
static inline int ti_sci_get_response(struct ti_sci_info *info,
				      struct ti_sci_xfer *xfer,
				      struct mbox_chan *chan)
{
	struct k3_sec_proxy_msg *msg = &xfer->tx_message;
	struct ti_sci_secure_msg_hdr *secure_hdr;
	struct ti_sci_msg_hdr *hdr;
	int ret;

	/* Receive the response */
	ret = mbox_recv(chan, msg, info->desc->max_rx_timeout_us);
	if (ret) {
		dev_err(info->dev, "%s: Message receive failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* ToDo: Verify checksum */
	if (info->is_secure) {
		secure_hdr = (struct ti_sci_secure_msg_hdr *)msg->buf;
		msg->buf = (u32 *)((void *)msg->buf + sizeof(*secure_hdr));
	}

	/* msg is updated by mailbox driver */
	hdr = (struct ti_sci_msg_hdr *)msg->buf;

	/* Sanity check for message response */
	if (hdr->seq != info->seq) {
		dev_dbg(info->dev, "%s: Message for %d is not expected\n",
			__func__, hdr->seq);
		return ret;
	}

	if (msg->len > info->desc->max_msg_size) {
		dev_err(info->dev, "%s: Unable to handle %zu xfer (max %d)\n",
			__func__, msg->len, info->desc->max_msg_size);
		return -EINVAL;
	}

	if (msg->len < xfer->rx_len) {
		dev_err(info->dev, "%s: Recv xfer %zu < expected %d length\n",
			__func__, msg->len, xfer->rx_len);
	}

	return ret;
}

/**
 * ti_sci_do_xfer() - Do one transfer
 * @info:	Pointer to SCI entity information
 * @xfer:	Transfer to initiate and wait for response
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static inline int ti_sci_do_xfer(struct ti_sci_info *info,
				 struct ti_sci_xfer *xfer)
{
	struct k3_sec_proxy_msg *msg = &xfer->tx_message;
	u8 secure_buf[info->desc->max_msg_size];
	struct ti_sci_secure_msg_hdr secure_hdr;
	int ret;

	if (info->is_secure) {
		/* ToDo: get checksum of the entire message */
		secure_hdr.checksum = 0;
		secure_hdr.reserved = 0;
		memcpy(&secure_buf[sizeof(secure_hdr)], xfer->tx_message.buf,
		       xfer->tx_message.len);

		xfer->tx_message.buf = (u32 *)secure_buf;
		xfer->tx_message.len += sizeof(secure_hdr);
		xfer->rx_len += sizeof(secure_hdr);
	}

	/* Send the message */
	ret = mbox_send(&info->chan_tx, msg);
	if (ret) {
		dev_err(info->dev, "%s: Message sending failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	return ti_sci_get_response(info, xfer, &info->chan_rx);
}

/**
 * ti_sci_cmd_get_revision() - command to get the revision of the SCI entity
 * @handle:	pointer to TI SCI handle
 *
 * Updates the SCI information in the internal data structure.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_revision(struct ti_sci_handle *handle)
{
	struct ti_sci_msg_resp_version *rev_info;
	struct ti_sci_version_info *ver;
	struct ti_sci_msg_hdr hdr;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_VERSION, 0x0,
				     (u32 *)&hdr, sizeof(struct ti_sci_msg_hdr),
				     sizeof(*rev_info));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox communication fail %d\n", ret);
		return ret;
	}

	rev_info = (struct ti_sci_msg_resp_version *)xfer->tx_message.buf;

	ver = &handle->version;
	ver->abi_major = rev_info->abi_major;
	ver->abi_minor = rev_info->abi_minor;
	ver->firmware_revision = rev_info->firmware_revision;
	strncpy(ver->firmware_description, rev_info->firmware_description,
		sizeof(ver->firmware_description));

	return 0;
}

/**
 * ti_sci_is_response_ack() - Generic ACK/NACK message checkup
 * @r:	pointer to response buffer
 *
 * Return: true if the response was an ACK, else returns false.
 */
static inline bool ti_sci_is_response_ack(void *r)
{
	struct ti_sci_msg_hdr *hdr = r;

	return hdr->flags & TI_SCI_FLAG_RESP_GENERIC_ACK ? true : false;
}

/**
 * cmd_set_board_config_using_msg() - Common command to send board configuration
 *                                    message
 * @handle:	pointer to TI SCI handle
 * @msg_type:	One of the TISCI message types to set board configuration
 * @addr:	Address where the board config structure is located
 * @size:	Size of the board config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int cmd_set_board_config_using_msg(const struct ti_sci_handle *handle,
					  u16 msg_type, u64 addr, u32 size)
{
	struct ti_sci_msg_board_config req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, msg_type,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.boardcfgp_high = (addr >> 32) & 0xffffffff;
	req.boardcfgp_low = addr & 0xffffffff;
	req.boardcfg_size = size;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_set_board_config() - Command to send board configuration message
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board config structure is located
 * @size:	Size of the board config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_board_config(const struct ti_sci_handle *handle,
				       u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG,
					      addr, size);
}

/**
 * ti_sci_cmd_set_board_config_rm() - Command to send board resource
 *				      management configuration
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board RM config structure is located
 * @size:	Size of the RM config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static
int ti_sci_cmd_set_board_config_rm(const struct ti_sci_handle *handle,
				   u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG_RM,
					      addr, size);
}

/**
 * ti_sci_cmd_set_board_config_security() - Command to send board security
 *					    configuration message
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board security config structure is located
 * @size:	Size of the security config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static
int ti_sci_cmd_set_board_config_security(const struct ti_sci_handle *handle,
					 u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG_SECURITY,
					      addr, size);
}

/**
 * ti_sci_cmd_set_board_config_pm() - Command to send board power and clock
 *				      configuration message
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board PM config structure is located
 * @size:	Size of the PM config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_board_config_pm(const struct ti_sci_handle *handle,
					  u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG_PM,
					      addr, size);
}

/*
 * ti_sci_setup_ops() - Setup the operations structures
 * @info:	pointer to TISCI pointer
 */
static void ti_sci_setup_ops(struct ti_sci_info *info)
{
	struct ti_sci_ops *ops = &info->handle.ops;
	struct ti_sci_board_ops *bops = &ops->board_ops;

	bops->board_config = ti_sci_cmd_set_board_config;
	bops->board_config_rm = ti_sci_cmd_set_board_config_rm;
	bops->board_config_security = ti_sci_cmd_set_board_config_security;
	bops->board_config_pm = ti_sci_cmd_set_board_config_pm;
}

/**
 * ti_sci_get_handle_from_sysfw() - Get the TI SCI handle of the SYSFW
 * @dev:	Pointer to the SYSFW device
 *
 * Return: pointer to handle if successful, else EINVAL if invalid conditions
 *         are encountered.
 */
const
struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *sci_dev)
{
	if (!sci_dev)
		return ERR_PTR(-EINVAL);

	struct ti_sci_info *info = dev_get_priv(sci_dev);

	if (!info)
		return ERR_PTR(-EINVAL);

	struct ti_sci_handle *handle = &info->handle;

	if (!handle)
		return ERR_PTR(-EINVAL);

	return handle;
}

/**
 * ti_sci_get_handle() - Get the TI SCI handle for a device
 * @dev:	Pointer to device for which we want SCI handle
 *
 * Return: pointer to handle if successful, else EINVAL if invalid conditions
 *         are encountered.
 */
const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev)
{
	if (!dev)
		return ERR_PTR(-EINVAL);

	struct udevice *sci_dev = dev_get_parent(dev);

	return ti_sci_get_handle_from_sysfw(sci_dev);
}

/**
 * ti_sci_get_by_phandle() - Get the TI SCI handle using DT phandle
 * @dev:	device node
 * @propname:	property name containing phandle on TISCI node
 *
 * Return: pointer to handle if successful, else appropriate error value.
 */
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property)
{
	struct ti_sci_info *entry, *info = NULL;
	u32 phandle, err;
	ofnode node;

	err = ofnode_read_u32(dev_ofnode(dev), property, &phandle);
	if (err)
		return ERR_PTR(err);

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node))
		return ERR_PTR(-EINVAL);

	list_for_each_entry(entry, &ti_sci_list, list)
		if (ofnode_equal(dev_ofnode(entry->dev), node)) {
			info = entry;
			break;
		}

	if (!info)
		return ERR_PTR(-ENODEV);

	return &info->handle;
}

/**
 * ti_sci_of_to_info() - generate private data from device tree
 * @dev:	corresponding system controller interface device
 * @info:	pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int ti_sci_of_to_info(struct udevice *dev, struct ti_sci_info *info)
{
	int ret;

	ret = mbox_get_by_name(dev, "tx", &info->chan_tx);
	if (ret) {
		dev_err(dev, "%s: Acquiring Tx channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &info->chan_rx);
	if (ret) {
		dev_err(dev, "%s: Acquiring Rx channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* Notify channel is optional. Enable only if populated */
	ret = mbox_get_by_name(dev, "notify", &info->chan_notify);
	if (ret) {
		dev_dbg(dev, "%s: Acquiring notify channel failed. ret = %d\n",
			__func__, ret);
	}

	info->host_id = dev_read_u32_default(dev, "ti,host-id",
					     info->desc->host_id);

	info->is_secure = dev_read_bool(dev, "ti,secure-host");

	return 0;
}

/**
 * ti_sci_probe() - Basic probe
 * @dev:	corresponding system controller interface device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int ti_sci_probe(struct udevice *dev)
{
	struct ti_sci_info *info;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	info = dev_get_priv(dev);
	info->desc = (void *)dev_get_driver_data(dev);

	ret = ti_sci_of_to_info(dev, info);
	if (ret) {
		dev_err(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	info->dev = dev;
	info->seq = 0xA;

	list_add_tail(&info->list, &ti_sci_list);
	ti_sci_setup_ops(info);

	ret = ti_sci_cmd_get_revision(&info->handle);

	return ret;
}

/* Description for AM654 */
static const struct ti_sci_desc ti_sci_sysfw_am654_desc = {
	.host_id = 4,
	.max_rx_timeout_us = 1000000,
	.max_msg_size = 60,
};

static const struct udevice_id ti_sci_ids[] = {
	{
		.compatible = "ti,k2g-sci",
		.data = (ulong)&ti_sci_sysfw_am654_desc
	},
	{ /* Sentinel */ },
};

U_BOOT_DRIVER(ti_sci) = {
	.name = "ti_sci",
	.id = UCLASS_FIRMWARE,
	.of_match = ti_sci_ids,
	.probe = ti_sci_probe,
	.priv_auto_alloc_size = sizeof(struct ti_sci_info),
};
