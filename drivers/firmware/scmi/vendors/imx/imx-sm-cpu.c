// SPDX-License-Identifier: GPL-2.0
/*
 * i.MX SCMI CPU protocol
 *
 * Copyright 2025 NXP
 */

#include <compiler.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <misc.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>
#include <scmi_nxp_protocols.h>

enum scmi_imx_cpu_protocol_cmd {
	SCMI_IMX_CPU_ATTRIBUTES	= 0x3,
	SCMI_IMX_CPU_START = 0x4,
	SCMI_IMX_CPU_STOP = 0x5,
	SCMI_IMX_CPU_RESET_VECTOR_SET = 0x6,
	SCMI_IMX_CPU_INFO_GET = 0xC,
};

struct scmi_imx_cpu_priv {
	u32 nr_cpu;
};

struct scmi_imx_cpu_reset_vector_set_in {
	__le32 cpuid;
#define	CPU_VEC_FLAGS_RESUME	BIT(31)
#define	CPU_VEC_FLAGS_START	BIT(30)
#define	CPU_VEC_FLAGS_BOOT	BIT(29)
	__le32 flags;
	__le32 resetvectorlow;
	__le32 resetvectorhigh;
};

struct scmi_imx_cpu_info_get_out {
	__le32 status;
#define	CPU_RUN_MODE_START	0
#define	CPU_RUN_MODE_HOLD	1
#define	CPU_RUN_MODE_STOP	2
#define	CPU_RUN_MODE_SLEEP	3
	__le32 runmode;
	__le32 sleepmode;
	__le32 resetvectorlow;
	__le32 resetvectorhigh;
};

int scmi_imx_cpu_start(struct udevice *dev, u32 cpuid, bool start)
{
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_82,
		.message_id = SCMI_IMX_CPU_STOP,
		.in_msg = (u8 *)&cpuid,
		.in_msg_sz = sizeof(cpuid),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	if (start)
		msg.message_id = SCMI_IMX_CPU_START;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

int scmi_imx_cpu_reset_vector_set(struct udevice *dev, u32 cpuid, u32 flags, u64 vector,
				  bool start, bool boot, bool resume)
{
	struct scmi_imx_cpu_reset_vector_set_in in;
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_82,
		.message_id = SCMI_IMX_CPU_RESET_VECTOR_SET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	in.cpuid = cpu_to_le32(cpuid);
	in.flags = cpu_to_le32(0);
	if (start)
		in.flags |= CPU_VEC_FLAGS_START;
	if (boot)
		in.flags |= CPU_VEC_FLAGS_BOOT;
	if (resume)
		in.flags |= CPU_VEC_FLAGS_RESUME;
	in.resetvectorlow = cpu_to_le32(lower_32_bits(vector));
	in.resetvectorhigh = cpu_to_le32(upper_32_bits(vector));

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

int scmi_imx_cpu_started(struct udevice *dev, u32 cpuid, bool *started)
{
	struct scmi_imx_cpu_info_get_out out;
	u32 mode;
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VENDOR_82,
		.message_id = SCMI_IMX_CPU_INFO_GET,
		.in_msg = (u8 *)&cpuid,
		.in_msg_sz = sizeof(cpuid),
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

	mode = le32_to_cpu(out.runmode);
	if (mode == CPU_RUN_MODE_START || mode == CPU_RUN_MODE_SLEEP)
		*started = true;

	return 0;
}

static int scmi_imx_cpu_probe(struct udevice *dev)
{
	int ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret) {
		dev_err(dev, "failed to get channel (%d)\n", ret);
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(scmi_imx_cpu) = {
	.name = "scmi_imx_cpu",
	.id = UCLASS_SCMI_BASE,
	.probe = scmi_imx_cpu_probe,
	.priv_auto = sizeof(struct scmi_imx_cpu_priv),
};

static struct scmi_proto_match match[] = {
	{ .proto_id = SCMI_PROTOCOL_ID_VENDOR_82},
	{ /* Sentinel */ }
};

U_BOOT_SCMI_PROTO_DRIVER(scmi_imx_cpu, match);
