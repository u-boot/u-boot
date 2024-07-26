// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/bug.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>

#include <soc/qcom/rpmh.h>

#include "rpmh-internal.h"

#define RPMH_TIMEOUT_MS			msecs_to_jiffies(10000)

#define DEFINE_RPMH_MSG_ONSTACK(device, s, name)	\
	struct rpmh_request name = {			\
		.msg = {				\
			.state = s,			\
			.cmds = name.cmd,		\
			.num_cmds = 0,			\
		},					\
		.cmd = { { 0 } },			\
		.dev = device,				\
		.needs_free = false,			\
	}

#define ctrlr_to_drv(ctrlr) container_of(ctrlr, struct rsc_drv, client)

static struct rpmh_ctrlr *get_rpmh_ctrlr(const struct udevice *dev)
{
	struct rsc_drv *drv = (struct rsc_drv *)dev_get_priv(dev->parent);

	if (!drv) {
		log_err("BUG: no RPMh driver for %s (parent %s)\n", dev->name, dev->parent->name);
		BUG();
	}

	return &drv->client;
}

/**
 * __rpmh_write: Cache and send the RPMH request
 *
 * @dev: The device making the request
 * @state: Active/Sleep request type
 * @rpm_msg: The data that needs to be sent (cmds).
 *
 * Cache the RPMH request and send if the state is ACTIVE_ONLY.
 * SLEEP/WAKE_ONLY requests are not sent to the controller at
 * this time. Use rpmh_flush() to send them to the controller.
 */
static int __rpmh_write(const struct udevice *dev, enum rpmh_state state,
			struct rpmh_request *rpm_msg)
{
	struct rpmh_ctrlr *ctrlr = get_rpmh_ctrlr(dev);

	if (state != RPMH_ACTIVE_ONLY_STATE) {
		log_err("only ACTIVE_ONLY state supported\n");
		return -EINVAL;
	}

	return rpmh_rsc_send_data(ctrlr_to_drv(ctrlr), &rpm_msg->msg);
}

static int __fill_rpmh_msg(struct rpmh_request *req, enum rpmh_state state,
			   const struct tcs_cmd *cmd, u32 n)
{
	if (!cmd || !n || n > MAX_RPMH_PAYLOAD)
		return -EINVAL;

	memcpy(req->cmd, cmd, n * sizeof(*cmd));

	req->msg.state = state;
	req->msg.cmds = req->cmd;
	req->msg.num_cmds = n;

	debug("rpmh_msg: %d, %d cmds [first %#x/%#x]\n", state, n, cmd->addr, cmd->data);

	return 0;
}

/**
 * rpmh_write: Write a set of RPMH commands and block until response
 *
 * @dev: The device making the request
 * @state: Active/sleep set
 * @cmd: The payload data
 * @n: The number of elements in @cmd
 *
 * May sleep. Do not call from atomic contexts.
 */
int rpmh_write(const struct udevice *dev, enum rpmh_state state,
	       const struct tcs_cmd *cmd, u32 n)
{
	DEFINE_RPMH_MSG_ONSTACK(dev, state, rpm_msg);
	int ret;

	ret = __fill_rpmh_msg(&rpm_msg, state, cmd, n);
	if (ret)
		return ret;

	ret = __rpmh_write(dev, state, &rpm_msg);

	return ret;
}
EXPORT_SYMBOL_GPL(rpmh_write);
