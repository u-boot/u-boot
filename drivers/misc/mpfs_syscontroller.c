// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip's PolarFire SoC (MPFS) System Controller Driver
 *
 * Copyright (C) 2024 Microchip Technology Inc. All rights reserved.
 *
 * Author: Jamie Gibbons <jamie.gibbons@microchip.com>
 *
 */

#include <asm/system.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <env.h>
#include <errno.h>
#include <linux/compat.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <log.h>
#include <mailbox.h>
#include <misc.h>
#include <mpfs-mailbox.h>

/* Descriptor table */
#define CMD_OPCODE						0x0u
#define CMD_DATA_SIZE					0U
#define CMD_DATA						NULL
#define MBOX_OFFSET						0x0
#define RESP_OFFSET						0x0
#define RESP_BYTES						16U

/**
 * struct mpfs_syscontroller_priv - Structure representing System Controller data.
 * @chan:	Mailbox channel
 * @c:	Completion signal
 */
struct mpfs_syscontroller_priv {
	struct mbox_chan chan;
	struct completion c;
};

/**
 * mpfs_syscontroller_run_service() - Run the MPFS system service
 * @sys_controller:	corresponding MPFS system service device
 * @msg:	Message to send
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
int mpfs_syscontroller_run_service(struct mpfs_syscontroller_priv *sys_controller, struct mpfs_mss_msg *msg)
{
	int ret;

	reinit_completion(&sys_controller->c);

	/* Run the System Service Request */
	ret = mbox_send(&sys_controller->chan, msg);
	if (ret < 0)
		dev_warn(sys_controller->chan.dev, "MPFS sys controller service timeout\n");

	debug("%s: Service successful %s\n",
	      __func__, sys_controller->chan.dev->name);

	return ret;
}
EXPORT_SYMBOL_GPL(mpfs_syscontroller_run_service);

/**
 * mpfs_syscontroller_read_sernum() - Use system service to read the device serial number
 * @sys_serv_priv:	system service private data
 * @device_serial_number:	device serial number
 *
 * Return: 0 if all went ok, else return appropriate error
 */
int mpfs_syscontroller_read_sernum(struct mpfs_sys_serv *sys_serv_priv, u8 *device_serial_number)
{
	unsigned long timeoutsecs = 300;
	int ret;

	struct mpfs_mss_response response = {
		.resp_status = 0U,
		.resp_msg = (u32 *)device_serial_number,
		.resp_size = RESP_BYTES};
	struct mpfs_mss_msg msg = {
		.cmd_opcode = CMD_OPCODE,
		.cmd_data_size = CMD_DATA_SIZE,
		.response = &response,
		.cmd_data = CMD_DATA,
		.mbox_offset = MBOX_OFFSET,
		.resp_offset = RESP_OFFSET};

	ret = mpfs_syscontroller_run_service(sys_serv_priv->sys_controller, &msg);
	if (ret) {
		dev_err(sys_serv_priv->sys_controller->chan.dev, "Service failed: %d, abort\n", ret);
		return ret;
	}

	/* Receive the response */
	ret = mbox_recv(&sys_serv_priv->sys_controller->chan, &msg, timeoutsecs);
	if (ret) {
		dev_err(sys_serv_priv->sys_controller->chan.dev, "Service failed: %d, abort. Failure: %u\n", ret, msg.response->resp_status);
		return ret;
	}

	debug("%s: Read successful %s\n",
	      __func__, sys_serv_priv->sys_controller->chan.dev->name);

	return 0;
}
EXPORT_SYMBOL(mpfs_syscontroller_read_sernum);

static int mpfs_syscontroller_probe(struct udevice *dev)
{
	struct mpfs_syscontroller_priv *sys_controller = dev_get_priv(dev);
	int ret;

	ret = mbox_get_by_index(dev, 0, &sys_controller->chan);
	if (ret) {
		dev_err(dev, "%s: Acquiring mailbox channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	init_completion(&sys_controller->c);
	dev_info(dev, "Registered MPFS system controller\n");

	return 0;
}

static const struct udevice_id mpfs_syscontroller_ids[] = {
	{ .compatible = "microchip,mpfs-sys-controller" },
	{ }
};

struct mpfs_syscontroller_priv *mpfs_syscontroller_get(struct udevice *dev)
{
	struct mpfs_syscontroller_priv *sys_controller;

	sys_controller = dev_get_priv(dev);
	if (!sys_controller) {
		debug("%s: MPFS system controller found but could not register as a sub device %p\n",
		      __func__, sys_controller);
		return ERR_PTR(-EPROBE_DEFER);
	}

	return sys_controller;
}
EXPORT_SYMBOL(mpfs_syscontroller_get);

U_BOOT_DRIVER(mpfs_syscontroller) = {
	.name           = "mpfs_syscontroller",
	.id             = UCLASS_MISC,
	.of_match       = mpfs_syscontroller_ids,
	.probe          = mpfs_syscontroller_probe,
	.priv_auto	= sizeof(struct mpfs_syscontroller_priv),
};
