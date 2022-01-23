// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020-2021 Linaro Limited.
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <string.h>
#include <tee.h>
#include <asm/types.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/arm-smccc.h>
#include <linux/bug.h>
#include <linux/compat.h>

#include "smt.h"

#define SCMI_SHM_SIZE		128

/**
 * struct scmi_optee_channel - Description of an SCMI OP-TEE transport
 * @channel_id:		Channel identifier
 * @smt:		Shared memory buffer with synchronisation protocol
 * @dyn_shm:		True if using dynamically allocated shared memory
 */
struct scmi_optee_channel {
	unsigned int channel_id;
	struct scmi_smt smt;
	bool dyn_shm;
};

/**
 * struct channel_session - Aggreates SCMI service session context references
 * @tee:		OP-TEE device to invoke
 * @tee_session:	OP-TEE session identifier
 * @tee_shm:		Dynamically allocated OP-TEE shared memory, or NULL
 * @channel_hdl:	Channel handle provided by OP-TEE SCMI service
 */
struct channel_session {
	struct udevice *tee;
	u32 tee_session;
	struct tee_shm *tee_shm;
	u32 channel_hdl;
};

#define TA_SCMI_UUID { 0xa8cfe406, 0xd4f5, 0x4a2e, \
		      { 0x9f, 0x8d, 0xa2, 0x5d, 0xc7, 0x54, 0xc0, 0x99 } }

enum optee_smci_pta_cmd {
	/*
	 * PTA_SCMI_CMD_CAPABILITIES - Get channel capabilities
	 *
	 * [out]    value[0].a: Capability bit mask (enum pta_scmi_caps)
	 * [out]    value[0].b: Extended capabilities or 0
	 */
	PTA_SCMI_CMD_CAPABILITIES = 0,

	/*
	 * PTA_SCMI_CMD_PROCESS_SMT_CHANNEL - Process SCMI message in SMT buffer
	 *
	 * [in]     value[0].a: Channel handle
	 *
	 * Shared memory used for SCMI message/response exhange is expected
	 * already identified and bound to channel handle in both SCMI agent
	 * and SCMI server (OP-TEE) parts.
	 * The memory uses SMT header to carry SCMI meta-data (protocol ID and
	 * protocol message ID).
	 */
	PTA_SCMI_CMD_PROCESS_SMT_CHANNEL = 1,

	/*
	 * PTA_SCMI_CMD_PROCESS_SMT_CHANNEL_MESSAGE - Process SMT/SCMI message
	 *
	 * [in]     value[0].a: Channel handle
	 * [in/out] memref[1]: Message/response buffer (SMT and SCMI payload)
	 *
	 * Shared memory used for SCMI message/response is a SMT buffer
	 * referenced by param[1]. It shall be 128 bytes large to fit response
	 * payload whatever message playload size.
	 * The memory uses SMT header to carry SCMI meta-data (protocol ID and
	 * protocol message ID).
	 */
	PTA_SCMI_CMD_PROCESS_SMT_CHANNEL_MESSAGE = 2,

	/*
	 * PTA_SCMI_CMD_GET_CHANNEL - Get channel handle
	 *
	 * SCMI shm information are 0 if agent expects to use OP-TEE regular SHM
	 *
	 * [in]     value[0].a: Channel identifier
	 * [out]    value[0].a: Returned channel handle
	 * [in]     value[0].b: Requested capabilities mask (enum pta_scmi_caps)
	 */
	PTA_SCMI_CMD_GET_CHANNEL = 3,
};

/*
 * OP-TEE SCMI service capabilities bit flags (32bit)
 *
 * PTA_SCMI_CAPS_SMT_HEADER
 * When set, OP-TEE supports command using SMT header protocol (SCMI shmem) in
 * shared memory buffers to carry SCMI protocol synchronisation information.
 */
#define PTA_SCMI_CAPS_NONE		0
#define PTA_SCMI_CAPS_SMT_HEADER	BIT(0)

static int open_channel(struct udevice *dev, struct channel_session *sess)
{
	const struct tee_optee_ta_uuid uuid = TA_SCMI_UUID;
	struct scmi_optee_channel *chan = dev_get_plat(dev);
	struct tee_open_session_arg sess_arg = { };
	struct tee_invoke_arg cmd_arg = { };
	struct tee_param param[1] = { };
	int ret;

	memset(sess, 0, sizeof(sess));

	sess->tee = tee_find_device(NULL, NULL, NULL, NULL);
	if (!sess->tee)
		return -ENODEV;

	sess_arg.clnt_login = TEE_LOGIN_REE_KERNEL;
	tee_optee_ta_uuid_to_octets(sess_arg.uuid, &uuid);

	ret = tee_open_session(sess->tee, &sess_arg, 0, NULL);
	if (ret) {
		dev_err(dev, "can't open session: %d\n", ret);
		return ret;
	}

	cmd_arg.func = PTA_SCMI_CMD_GET_CHANNEL;
	cmd_arg.session = sess_arg.session;

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INOUT;
	param[0].u.value.a = chan->channel_id;
	param[0].u.value.b = PTA_SCMI_CAPS_SMT_HEADER;

	ret = tee_invoke_func(sess->tee, &cmd_arg, ARRAY_SIZE(param), param);
	if (ret || cmd_arg.ret) {
		dev_err(dev, "Invoke failed: %d, 0x%x\n", ret, cmd_arg.ret);
		if (!ret)
			ret = -EPROTO;

		tee_close_session(sess->tee, sess_arg.session);
		return ret;
	}

	sess->tee_session = sess_arg.session;
	sess->channel_hdl = param[0].u.value.a;

	return 0;
}

static void close_channel(struct channel_session *sess)
{
	tee_close_session(sess->tee, sess->tee_session);
}

static int invoke_cmd(struct udevice *dev, struct channel_session *sess,
		      struct scmi_msg *msg)
{
	struct scmi_optee_channel *chan = dev_get_plat(dev);
	struct tee_invoke_arg arg = { };
	struct tee_param param[2] = { };
	int ret;

	scmi_write_msg_to_smt(dev, &chan->smt, msg);

	arg.session = sess->tee_session;
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = sess->channel_hdl;

	if (chan->dyn_shm) {
		arg.func = PTA_SCMI_CMD_PROCESS_SMT_CHANNEL_MESSAGE;
		param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
		param[1].u.memref.shm = sess->tee_shm;
		param[1].u.memref.size = SCMI_SHM_SIZE;
	} else {
		arg.func = PTA_SCMI_CMD_PROCESS_SMT_CHANNEL;
	}

	ret = tee_invoke_func(sess->tee, &arg, ARRAY_SIZE(param), param);
	if (ret || arg.ret) {
		if (!ret)
			ret = -EPROTO;
	} else {
		ret = scmi_read_resp_from_smt(dev, &chan->smt, msg);
	}

	scmi_clear_smt_channel(&chan->smt);

	return ret;
}

static int prepare_shm(struct udevice *dev, struct channel_session *sess)
{
	struct scmi_optee_channel *chan = dev_get_plat(dev);
	int ret;

	/* Static shm is already prepared by the firmware: nothing to do */
	if (!chan->dyn_shm)
		return 0;

	chan->smt.size = SCMI_SHM_SIZE;

	ret = tee_shm_alloc(sess->tee, chan->smt.size, 0, &sess->tee_shm);
	if (ret) {
		dev_err(dev, "Failed to allocated shmem: %d\n", ret);
		return ret;
	}

	chan->smt.buf = sess->tee_shm->addr;

	/* Initialize shm buffer for message exchanges */
	scmi_clear_smt_channel(&chan->smt);

	return 0;
}

static void release_shm(struct udevice *dev, struct channel_session *sess)
{
	struct scmi_optee_channel *chan = dev_get_plat(dev);

	if (chan->dyn_shm)
		tee_shm_free(sess->tee_shm);
}

static int scmi_optee_process_msg(struct udevice *dev, struct scmi_msg *msg)
{
	struct channel_session sess;
	int ret;

	ret = open_channel(dev, &sess);
	if (ret)
		return ret;

	ret = prepare_shm(dev, &sess);
	if (ret)
		goto out;

	ret = invoke_cmd(dev, &sess, msg);

	release_shm(dev, &sess);

out:
	close_channel(&sess);

	return ret;
}

static int scmi_optee_of_to_plat(struct udevice *dev)
{
	struct scmi_optee_channel *chan = dev_get_plat(dev);
	int ret;

	if (dev_read_u32(dev, "linaro,optee-channel-id", &chan->channel_id)) {
		dev_err(dev, "Missing property linaro,optee-channel-id\n");
		return -EINVAL;
	}

	if (dev_read_prop(dev, "shmem", NULL)) {
		ret = scmi_dt_get_smt_buffer(dev, &chan->smt);
		if (ret) {
			dev_err(dev, "Failed to get smt resources: %d\n", ret);
			return ret;
		}
		chan->dyn_shm = false;
	} else {
		chan->dyn_shm = true;
	}

	return 0;
}

static int scmi_optee_probe(struct udevice *dev)
{
	struct channel_session sess;
	int ret;

	/* Check OP-TEE service acknowledges the SCMI channel */
	ret = open_channel(dev, &sess);
	if (!ret)
		close_channel(&sess);

	return ret;
}

static const struct udevice_id scmi_optee_ids[] = {
	{ .compatible = "linaro,scmi-optee" },
	{ }
};

static const struct scmi_agent_ops scmi_optee_ops = {
	.process_msg = scmi_optee_process_msg,
};

U_BOOT_DRIVER(scmi_optee) = {
	.name		= "scmi-over-optee",
	.id		= UCLASS_SCMI_AGENT,
	.of_match	= scmi_optee_ids,
	.plat_auto	= sizeof(struct scmi_optee_channel),
	.of_to_plat	= scmi_optee_of_to_plat,
	.probe		= scmi_optee_probe,
	.flags		= DM_FLAG_OS_PREPARE,
	.ops		= &scmi_optee_ops,
};
