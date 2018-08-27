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

/**
 * ti_sci_set_device_state() - Set device state helper
 * @handle:	pointer to TI SCI handle
 * @id:		Device identifier
 * @flags:	flags to setup for the device
 * @state:	State to move the device to
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_set_device_state(const struct ti_sci_handle *handle,
				   u32 id, u32 flags, u8 state)
{
	struct ti_sci_msg_req_set_device_state req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_DEVICE_STATE,
				     flags | TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.id = id;
	req.state = state;

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
 * ti_sci_get_device_state() - Get device state helper
 * @handle:	Handle to the device
 * @id:		Device Identifier
 * @clcnt:	Pointer to Context Loss Count
 * @resets:	pointer to resets
 * @p_state:	pointer to p_state
 * @c_state:	pointer to c_state
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_get_device_state(const struct ti_sci_handle *handle,
				   u32 id,  u32 *clcnt,  u32 *resets,
				   u8 *p_state,  u8 *c_state)
{
	struct ti_sci_msg_resp_get_device_state *resp;
	struct ti_sci_msg_req_get_device_state req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	if (!clcnt && !resets && !p_state && !c_state)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	/* Response is expected, so need of any flags */
	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_DEVICE_STATE, 0,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.id = id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_device_state *)xfer->tx_message.buf;
	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	if (clcnt)
		*clcnt = resp->context_loss_count;
	if (resets)
		*resets = resp->resets;
	if (p_state)
		*p_state = resp->programmed_state;
	if (c_state)
		*c_state = resp->current_state;

	return ret;
}

/**
 * ti_sci_cmd_get_device() - command to request for device managed by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * NOTE: The request is for exclusive access for the processor.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_device(const struct ti_sci_handle *handle, u32 id)
{
	return ti_sci_set_device_state(handle, id,
				       MSG_FLAG_DEVICE_EXCLUSIVE,
				       MSG_DEVICE_SW_STATE_ON);
}

/**
 * ti_sci_cmd_idle_device() - Command to idle a device managed by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_idle_device(const struct ti_sci_handle *handle, u32 id)
{
	return ti_sci_set_device_state(handle, id,
				       MSG_FLAG_DEVICE_EXCLUSIVE,
				       MSG_DEVICE_SW_STATE_RETENTION);
}

/**
 * ti_sci_cmd_put_device() - command to release a device managed by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_put_device(const struct ti_sci_handle *handle, u32 id)
{
	return ti_sci_set_device_state(handle, id,
				       0, MSG_DEVICE_SW_STATE_AUTO_OFF);
}

/**
 * ti_sci_cmd_dev_is_valid() - Is the device valid
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Return: 0 if all went fine and the device ID is valid, else return
 * appropriate error.
 */
static int ti_sci_cmd_dev_is_valid(const struct ti_sci_handle *handle, u32 id)
{
	u8 unused;

	/* check the device state which will also tell us if the ID is valid */
	return ti_sci_get_device_state(handle, id, NULL, NULL, NULL, &unused);
}

/**
 * ti_sci_cmd_dev_get_clcnt() - Get context loss counter
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @count:	Pointer to Context Loss counter to populate
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_get_clcnt(const struct ti_sci_handle *handle, u32 id,
				    u32 *count)
{
	return ti_sci_get_device_state(handle, id, count, NULL, NULL, NULL);
}

/**
 * ti_sci_cmd_dev_is_idle() - Check if the device is requested to be idle
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @r_state:	true if requested to be idle
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_idle(const struct ti_sci_handle *handle, u32 id,
				  bool *r_state)
{
	int ret;
	u8 state;

	if (!r_state)
		return -EINVAL;

	ret = ti_sci_get_device_state(handle, id, NULL, NULL, &state, NULL);
	if (ret)
		return ret;

	*r_state = (state == MSG_DEVICE_SW_STATE_RETENTION);

	return 0;
}

/**
 * ti_sci_cmd_dev_is_stop() - Check if the device is requested to be stopped
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @r_state:	true if requested to be stopped
 * @curr_state:	true if currently stopped.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_stop(const struct ti_sci_handle *handle, u32 id,
				  bool *r_state,  bool *curr_state)
{
	int ret;
	u8 p_state, c_state;

	if (!r_state && !curr_state)
		return -EINVAL;

	ret =
	    ti_sci_get_device_state(handle, id, NULL, NULL, &p_state, &c_state);
	if (ret)
		return ret;

	if (r_state)
		*r_state = (p_state == MSG_DEVICE_SW_STATE_AUTO_OFF);
	if (curr_state)
		*curr_state = (c_state == MSG_DEVICE_HW_STATE_OFF);

	return 0;
}

/**
 * ti_sci_cmd_dev_is_on() - Check if the device is requested to be ON
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @r_state:	true if requested to be ON
 * @curr_state:	true if currently ON and active
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_on(const struct ti_sci_handle *handle, u32 id,
				bool *r_state,  bool *curr_state)
{
	int ret;
	u8 p_state, c_state;

	if (!r_state && !curr_state)
		return -EINVAL;

	ret =
	    ti_sci_get_device_state(handle, id, NULL, NULL, &p_state, &c_state);
	if (ret)
		return ret;

	if (r_state)
		*r_state = (p_state == MSG_DEVICE_SW_STATE_ON);
	if (curr_state)
		*curr_state = (c_state == MSG_DEVICE_HW_STATE_ON);

	return 0;
}

/**
 * ti_sci_cmd_dev_is_trans() - Check if the device is currently transitioning
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @curr_state:	true if currently transitioning.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_trans(const struct ti_sci_handle *handle, u32 id,
				   bool *curr_state)
{
	int ret;
	u8 state;

	if (!curr_state)
		return -EINVAL;

	ret = ti_sci_get_device_state(handle, id, NULL, NULL, NULL, &state);
	if (ret)
		return ret;

	*curr_state = (state == MSG_DEVICE_HW_STATE_TRANS);

	return 0;
}

/**
 * ti_sci_cmd_set_device_resets() - command to set resets for device managed
 *				    by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 * @reset_state: Device specific reset bit field
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_set_device_resets(const struct ti_sci_handle *handle,
					u32 id, u32 reset_state)
{
	struct ti_sci_msg_req_set_device_resets req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_DEVICE_RESETS,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.id = id;
	req.resets = reset_state;

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
 * ti_sci_cmd_get_device_resets() - Get reset state for device managed
 *				    by TISCI
 * @handle:		Pointer to TISCI handle
 * @id:			Device Identifier
 * @reset_state:	Pointer to reset state to populate
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_device_resets(const struct ti_sci_handle *handle,
					u32 id, u32 *reset_state)
{
	return ti_sci_get_device_state(handle, id, NULL, reset_state, NULL,
				       NULL);
}

/**
 * ti_sci_set_clock_state() - Set clock state helper
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @flags:	Header flags as needed
 * @state:	State to request for the clock.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_set_clock_state(const struct ti_sci_handle *handle,
				  u32 dev_id, u8 clk_id,
				  u32 flags, u8 state)
{
	struct ti_sci_msg_req_set_clock_state req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_CLOCK_STATE,
				     flags | TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.request_state = state;

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
 * ti_sci_cmd_get_clock_state() - Get clock state helper
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @programmed_state:	State requested for clock to move to
 * @current_state:	State that the clock is currently in
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_clock_state(const struct ti_sci_handle *handle,
				      u32 dev_id, u8 clk_id,
				      u8 *programmed_state, u8 *current_state)
{
	struct ti_sci_msg_resp_get_clock_state *resp;
	struct ti_sci_msg_req_get_clock_state req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	if (!programmed_state && !current_state)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_CLOCK_STATE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_state *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	if (programmed_state)
		*programmed_state = resp->programmed_state;
	if (current_state)
		*current_state = resp->current_state;

	return ret;
}

/**
 * ti_sci_cmd_get_clock() - Get control of a clock from TI SCI
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @needs_ssc: 'true' if Spread Spectrum clock is desired, else 'false'
 * @can_change_freq: 'true' if frequency change is desired, else 'false'
 * @enable_input_term: 'true' if input termination is desired, else 'false'
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_clock(const struct ti_sci_handle *handle, u32 dev_id,
				u8 clk_id, bool needs_ssc, bool can_change_freq,
				bool enable_input_term)
{
	u32 flags = 0;

	flags |= needs_ssc ? MSG_FLAG_CLOCK_ALLOW_SSC : 0;
	flags |= can_change_freq ? MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE : 0;
	flags |= enable_input_term ? MSG_FLAG_CLOCK_INPUT_TERM : 0;

	return ti_sci_set_clock_state(handle, dev_id, clk_id, flags,
				      MSG_CLOCK_SW_STATE_REQ);
}

/**
 * ti_sci_cmd_idle_clock() - Idle a clock which is in our control
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 *
 * NOTE: This clock must have been requested by get_clock previously.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_idle_clock(const struct ti_sci_handle *handle,
				 u32 dev_id, u8 clk_id)
{
	return ti_sci_set_clock_state(handle, dev_id, clk_id, 0,
				      MSG_CLOCK_SW_STATE_UNREQ);
}

/**
 * ti_sci_cmd_put_clock() - Release a clock from our control back to TISCI
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 *
 * NOTE: This clock must have been requested by get_clock previously.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_put_clock(const struct ti_sci_handle *handle,
				u32 dev_id, u8 clk_id)
{
	return ti_sci_set_clock_state(handle, dev_id, clk_id, 0,
				      MSG_CLOCK_SW_STATE_AUTO);
}

/**
 * ti_sci_cmd_clk_is_auto() - Is the clock being auto managed
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is auto managed
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_is_auto(const struct ti_sci_handle *handle,
				  u32 dev_id, u8 clk_id, bool *req_state)
{
	u8 state = 0;
	int ret;

	if (!req_state)
		return -EINVAL;

	ret = ti_sci_cmd_get_clock_state(handle, dev_id, clk_id, &state, NULL);
	if (ret)
		return ret;

	*req_state = (state == MSG_CLOCK_SW_STATE_AUTO);
	return 0;
}

/**
 * ti_sci_cmd_clk_is_on() - Is the clock ON
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is managed by us and enabled
 * @curr_state: state indicating if the clock is ready for operation
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_is_on(const struct ti_sci_handle *handle, u32 dev_id,
				u8 clk_id, bool *req_state, bool *curr_state)
{
	u8 c_state = 0, r_state = 0;
	int ret;

	if (!req_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_cmd_get_clock_state(handle, dev_id, clk_id,
					 &r_state, &c_state);
	if (ret)
		return ret;

	if (req_state)
		*req_state = (r_state == MSG_CLOCK_SW_STATE_REQ);
	if (curr_state)
		*curr_state = (c_state == MSG_CLOCK_HW_STATE_READY);
	return 0;
}

/**
 * ti_sci_cmd_clk_is_off() - Is the clock OFF
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is managed by us and disabled
 * @curr_state: state indicating if the clock is NOT ready for operation
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_is_off(const struct ti_sci_handle *handle, u32 dev_id,
				 u8 clk_id, bool *req_state, bool *curr_state)
{
	u8 c_state = 0, r_state = 0;
	int ret;

	if (!req_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_cmd_get_clock_state(handle, dev_id, clk_id,
					 &r_state, &c_state);
	if (ret)
		return ret;

	if (req_state)
		*req_state = (r_state == MSG_CLOCK_SW_STATE_UNREQ);
	if (curr_state)
		*curr_state = (c_state == MSG_CLOCK_HW_STATE_NOT_READY);
	return 0;
}

/**
 * ti_sci_cmd_clk_set_parent() - Set the clock source of a specific device clock
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	Parent clock identifier to set
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_set_parent(const struct ti_sci_handle *handle,
				     u32 dev_id, u8 clk_id, u8 parent_id)
{
	struct ti_sci_msg_req_set_clock_parent req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_CLOCK_PARENT,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.parent_id = parent_id;

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
 * ti_sci_cmd_clk_get_parent() - Get current parent clock source
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	Current clock parent
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_parent(const struct ti_sci_handle *handle,
				     u32 dev_id, u8 clk_id, u8 *parent_id)
{
	struct ti_sci_msg_resp_get_clock_parent *resp;
	struct ti_sci_msg_req_get_clock_parent req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !parent_id)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_CLOCK_PARENT,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_parent *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*parent_id = resp->parent_id;

	return ret;
}

/**
 * ti_sci_cmd_clk_get_num_parents() - Get num parents of the current clk source
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @num_parents: Returns he number of parents to the current clock.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_num_parents(const struct ti_sci_handle *handle,
					  u32 dev_id, u8 clk_id,
					  u8 *num_parents)
{
	struct ti_sci_msg_resp_get_clock_num_parents *resp;
	struct ti_sci_msg_req_get_clock_num_parents req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !num_parents)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_NUM_CLOCK_PARENTS,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_num_parents *)
							xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*num_parents = resp->num_parents;

	return ret;
}

/**
 * ti_sci_cmd_clk_get_match_freq() - Find a good match for frequency
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @match_freq:	Frequency match in Hz response.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_match_freq(const struct ti_sci_handle *handle,
					 u32 dev_id, u8 clk_id, u64 min_freq,
					 u64 target_freq, u64 max_freq,
					 u64 *match_freq)
{
	struct ti_sci_msg_resp_query_clock_freq *resp;
	struct ti_sci_msg_req_query_clock_freq req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !match_freq)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_QUERY_CLOCK_FREQ,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.min_freq_hz = min_freq;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = max_freq;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_query_clock_freq *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*match_freq = resp->freq_hz;

	return ret;
}

/**
 * ti_sci_cmd_clk_set_freq() - Set a frequency for clock
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_set_freq(const struct ti_sci_handle *handle,
				   u32 dev_id, u8 clk_id, u64 min_freq,
				   u64 target_freq, u64 max_freq)
{
	struct ti_sci_msg_req_set_clock_freq req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_CLOCK_FREQ,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.min_freq_hz = min_freq;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = max_freq;

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
 * ti_sci_cmd_clk_get_freq() - Get current frequency
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @freq:	Currently frequency in Hz
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_freq(const struct ti_sci_handle *handle,
				   u32 dev_id, u8 clk_id, u64 *freq)
{
	struct ti_sci_msg_resp_get_clock_freq *resp;
	struct ti_sci_msg_req_get_clock_freq req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !freq)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_CLOCK_FREQ,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_freq *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*freq = resp->freq_hz;

	return ret;
}

/**
 * ti_sci_cmd_core_reboot() - Command to request system reset
 * @handle:	pointer to TI SCI handle
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_core_reboot(const struct ti_sci_handle *handle)
{
	struct ti_sci_msg_req_reboot req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SYS_RESET,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_request() - Command to request a physical processor control
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_request(const struct ti_sci_handle *handle,
				   u8 proc_id)
{
	struct ti_sci_msg_req_proc_request req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_REQUEST,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_release() - Command to release a physical processor control
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_release(const struct ti_sci_handle *handle,
				   u8 proc_id)
{
	struct ti_sci_msg_req_proc_release req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_RELEASE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_handover() - Command to handover a physical processor
 *				control to a host in the processor's access
 *				control list.
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 * @host_id:	Host ID to get the control of the processor
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_handover(const struct ti_sci_handle *handle,
				    u8 proc_id, u8 host_id)
{
	struct ti_sci_msg_req_proc_handover req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_HANDOVER,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.host_id = host_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_set_proc_boot_cfg() - Command to set the processor boot
 *				    configuration flags
 * @handle:		Pointer to TI SCI handle
 * @proc_id:		Processor ID this request is for
 * @config_flags_set:	Configuration flags to be set
 * @config_flags_clear:	Configuration flags to be cleared.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_proc_boot_cfg(const struct ti_sci_handle *handle,
					u8 proc_id, u64 bootvector,
					u32 config_flags_set,
					u32 config_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_config req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_SET_PROC_BOOT_CONFIG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.bootvector_low = bootvector & TISCI_ADDR_LOW_MASK;
	req.bootvector_high = (bootvector & TISCI_ADDR_HIGH_MASK) >>
				TISCI_ADDR_HIGH_SHIFT;
	req.config_flags_set = config_flags_set;
	req.config_flags_clear = config_flags_clear;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_set_proc_boot_ctrl() - Command to set the processor boot
 *				     control flags
 * @handle:			Pointer to TI SCI handle
 * @proc_id:			Processor ID this request is for
 * @control_flags_set:		Control flags to be set
 * @control_flags_clear:	Control flags to be cleared
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_proc_boot_ctrl(const struct ti_sci_handle *handle,
					 u8 proc_id, u32 control_flags_set,
					 u32 control_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_ctrl req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_SET_PROC_BOOT_CTRL,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.control_flags_set = control_flags_set;
	req.control_flags_clear = control_flags_clear;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_auth_boot_image() - Command to authenticate and load the
 *			image and then set the processor configuration flags.
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 * @cert_addr:	Memory address at which payload image certificate is located.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_auth_boot_image(const struct ti_sci_handle *handle,
					   u8 proc_id, u64 cert_addr)
{
	struct ti_sci_msg_req_proc_auth_boot_image req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_AUTH_BOOT_IMIAGE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.cert_addr_low = cert_addr & TISCI_ADDR_LOW_MASK;
	req.cert_addr_high = (cert_addr & TISCI_ADDR_HIGH_MASK) >>
				TISCI_ADDR_HIGH_SHIFT;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_get_proc_boot_status() - Command to get the processor boot status
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_proc_boot_status(const struct ti_sci_handle *handle,
					   u8 proc_id, u64 *bv, u32 *cfg_flags,
					   u32 *ctrl_flags, u32 *sts_flags)
{
	struct ti_sci_msg_resp_get_proc_boot_status *resp;
	struct ti_sci_msg_req_get_proc_boot_status req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_GET_PROC_BOOT_STATUS,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_proc_boot_status *)
							xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;
	*bv = (resp->bootvector_low & TISCI_ADDR_LOW_MASK) |
			(((u64)resp->bootvector_high  <<
			  TISCI_ADDR_HIGH_SHIFT) & TISCI_ADDR_HIGH_MASK);
	*cfg_flags = resp->config_flags;
	*ctrl_flags = resp->control_flags;
	*sts_flags = resp->status_flags;

	return ret;
}

/*
 * ti_sci_setup_ops() - Setup the operations structures
 * @info:	pointer to TISCI pointer
 */
static void ti_sci_setup_ops(struct ti_sci_info *info)
{
	struct ti_sci_ops *ops = &info->handle.ops;
	struct ti_sci_board_ops *bops = &ops->board_ops;
	struct ti_sci_dev_ops *dops = &ops->dev_ops;
	struct ti_sci_clk_ops *cops = &ops->clk_ops;
	struct ti_sci_core_ops *core_ops = &ops->core_ops;
	struct ti_sci_proc_ops *pops = &ops->proc_ops;

	bops->board_config = ti_sci_cmd_set_board_config;
	bops->board_config_rm = ti_sci_cmd_set_board_config_rm;
	bops->board_config_security = ti_sci_cmd_set_board_config_security;
	bops->board_config_pm = ti_sci_cmd_set_board_config_pm;

	dops->get_device = ti_sci_cmd_get_device;
	dops->idle_device = ti_sci_cmd_idle_device;
	dops->put_device = ti_sci_cmd_put_device;
	dops->is_valid = ti_sci_cmd_dev_is_valid;
	dops->get_context_loss_count = ti_sci_cmd_dev_get_clcnt;
	dops->is_idle = ti_sci_cmd_dev_is_idle;
	dops->is_stop = ti_sci_cmd_dev_is_stop;
	dops->is_on = ti_sci_cmd_dev_is_on;
	dops->is_transitioning = ti_sci_cmd_dev_is_trans;
	dops->set_device_resets = ti_sci_cmd_set_device_resets;
	dops->get_device_resets = ti_sci_cmd_get_device_resets;

	cops->get_clock = ti_sci_cmd_get_clock;
	cops->idle_clock = ti_sci_cmd_idle_clock;
	cops->put_clock = ti_sci_cmd_put_clock;
	cops->is_auto = ti_sci_cmd_clk_is_auto;
	cops->is_on = ti_sci_cmd_clk_is_on;
	cops->is_off = ti_sci_cmd_clk_is_off;

	cops->set_parent = ti_sci_cmd_clk_set_parent;
	cops->get_parent = ti_sci_cmd_clk_get_parent;
	cops->get_num_parents = ti_sci_cmd_clk_get_num_parents;

	cops->get_best_match_freq = ti_sci_cmd_clk_get_match_freq;
	cops->set_freq = ti_sci_cmd_clk_set_freq;
	cops->get_freq = ti_sci_cmd_clk_get_freq;

	core_ops->reboot_device = ti_sci_cmd_core_reboot;

	pops->proc_request = ti_sci_cmd_proc_request;
	pops->proc_release = ti_sci_cmd_proc_release;
	pops->proc_handover = ti_sci_cmd_proc_handover;
	pops->set_proc_boot_cfg = ti_sci_cmd_set_proc_boot_cfg;
	pops->set_proc_boot_ctrl = ti_sci_cmd_set_proc_boot_ctrl;
	pops->proc_auth_boot_image = ti_sci_cmd_proc_auth_boot_image;
	pops->get_proc_boot_status = ti_sci_cmd_get_proc_boot_status;
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
