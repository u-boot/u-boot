// SPDX-License-Identifier: GPL-2.0
/*
 * i.MX SCMI LMM protocol
 *
 * Copyright 2025 NXP
 */

#include <compiler.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/types.h>
#include <misc.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>
#include <scmi_nxp_protocols.h>

enum scmi_imx_lmm_protocol_cmd {
	SCMI_IMX_LMM_ATTRIBUTES	= 0x3,
	SCMI_IMX_LMM_BOOT = 0x4,
	SCMI_IMX_LMM_RESET = 0x5,
	SCMI_IMX_LMM_SHUTDOWN = 0x6,
	SCMI_IMX_LMM_WAKE = 0x7,
	SCMI_IMX_LMM_SUSPEND = 0x8,
	SCMI_IMX_LMM_NOTIFY = 0x9,
	SCMI_IMX_LMM_RESET_REASON = 0xA,
	SCMI_IMX_LMM_POWER_ON = 0xB,
	SCMI_IMX_LMM_RESET_VECTOR_SET = 0xC,
};

struct scmi_imx_lmm_priv {
	u32 nr_lmm;
};

struct scmi_msg_imx_lmm_attributes_out {
	__le32 status;
	__le32 lmid;
	__le32 attributes;
	__le32 state;
	__le32 errstatus;
	u8 name[LMM_MAX_NAME];
};

struct scmi_imx_lmm_reset_vector_set_in {
	__le32 lmid;
	__le32 cpuid;
	__le32 flags; /* reserved for future extension */
	__le32 resetvectorlow;
	__le32 resetvectorhigh;
};

struct scmi_imx_lmm_shutdown_in {
	__le32 lmid;
#define SCMI_IMX_LMM_SHUTDOWN_GRACEFUL  BIT(0)
	__le32 flags;
};

int scmi_imx_lmm_info(struct udevice *dev, u32 lmid, struct scmi_imx_lmm_info *info)
{
	struct scmi_msg_imx_lmm_attributes_out out;
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_80,
		.message_id = SCMI_IMX_LMM_ATTRIBUTES,
		.in_msg = (u8 *)&lmid,
		.in_msg_sz = sizeof(lmid),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	status = cpu_to_le32(out.status);
	if (status)
		return scmi_to_linux_errno(status);

	info->lmid = le32_to_cpu(out.lmid);
	info->state = le32_to_cpu(out.state);
	info->errstatus = le32_to_cpu(out.errstatus);
	strcpy(info->name, out.name);
	dev_dbg(dev, "i.MX LMM: Logical Machine(%d), name: %s\n",
		info->lmid, info->name);

	return ret;
}

int scmi_imx_lmm_power_boot(struct udevice *dev, u32 lmid, bool boot)
{
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_80,
		.message_id = SCMI_IMX_LMM_POWER_ON,
		.in_msg = (u8 *)&lmid,
		.in_msg_sz = sizeof(lmid),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	if (boot)
		msg.message_id = SCMI_IMX_LMM_BOOT;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

int scmi_imx_lmm_reset_vector_set(struct udevice *dev, u32 lmid, u32 cpuid, u32 flags, u64 vector)
{
	struct scmi_imx_lmm_reset_vector_set_in in;
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_80,
		.message_id = SCMI_IMX_LMM_RESET_VECTOR_SET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	in.lmid = lmid;
	in.cpuid = cpuid;
	in.flags = flags;
	in.resetvectorlow = vector & 0xFFFFFFFF;
	in.resetvectorhigh = vector >> 32;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

int scmi_imx_lmm_shutdown(struct udevice *dev, u32 lmid, bool flags)
{
	struct scmi_imx_lmm_shutdown_in in;
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_80,
		.message_id = SCMI_IMX_LMM_SHUTDOWN,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	in.lmid = lmid;
	if (flags & SCMI_IMX_LMM_SHUTDOWN_GRACEFUL)
		in.flags = cpu_to_le32(SCMI_IMX_LMM_SHUTDOWN_GRACEFUL);
	else
		in.flags = cpu_to_le32(0);

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

static int scmi_imx_lmm_probe(struct udevice *dev)
{
	int ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret) {
		dev_err(dev, "failed to get channel (%d)\n", ret);
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(scmi_imx_lmm) = {
	.name = "scmi_imx_lmm",
	.id = UCLASS_SCMI_BASE,
	.probe = scmi_imx_lmm_probe,
	.priv_auto = sizeof(struct scmi_imx_lmm_priv),
};

static struct scmi_proto_match match[] = {
	{ .proto_id = SCMI_PROTOCOL_ID_VENDOR_80},
	{ /* Sentinel */ }
};

U_BOOT_SCMI_PROTO_DRIVER(scmi_imx_lmm, match);
