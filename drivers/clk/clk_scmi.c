// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-2022 Linaro Limited
 */

#define LOG_CATEGORY UCLASS_CLK

#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>
#include <asm/types.h>
#include <linux/clk-provider.h>

struct clk_scmi {
	struct clk clk;
	char name[SCMI_CLOCK_NAME_LENGTH_MAX];
	u32 ctrl_flags;
	bool attrs_resolved;
};

struct scmi_clock_priv {
	u32 version;
};

static int scmi_clk_get_permissions(struct udevice *dev, int clkid, u32 *perm)
{
	struct scmi_clock_priv *priv = dev_get_priv(dev);
	int ret;

	struct scmi_clk_get_permissions_in in = {
		.clock_id = clkid,
	};
	struct scmi_clk_get_permissions_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_CLOCK_GET_PERMISSIONS,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};

	if (priv->version < CLOCK_PROTOCOL_VERSION_3_0) {
		dev_dbg(dev,
			"%s: SCMI clock management protocol version is less than 3.0.\n", __func__);
		return -EINVAL;
	}

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		dev_dbg(dev,
			"%s: get SCMI clock management protocol permissions failed\n", __func__);
		return ret;
	}

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0) {
		dev_dbg(dev, "%s: the status code of getting permissions: %d\n", __func__, ret);
		return ret;
	}

	*perm = out.permissions;
	return 0;
}

static int scmi_clk_get_num_clock(struct udevice *dev, size_t *num_clocks)
{
	struct scmi_clk_protocol_attr_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_PROTOCOL_ATTRIBUTES,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	*num_clocks = out.attributes & SCMI_CLK_PROTO_ATTR_COUNT_MASK;

	return 0;
}

static int scmi_clk_get_attribute(struct udevice *dev, int clkid, char *name,
				  u32 *attr)
{
	struct scmi_clock_priv *priv = dev_get_priv(dev);
	struct scmi_clk_attribute_in in = {
		.clock_id = clkid,
	};
	int ret;

	if (priv->version >= 0x20000) {
		struct scmi_clk_attribute_out_v2 out;
		struct scmi_msg msg = {
			.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
			.message_id = SCMI_CLOCK_ATTRIBUTES,
			.in_msg = (u8 *)&in,
			.in_msg_sz = sizeof(in),
			.out_msg = (u8 *)&out,
			.out_msg_sz = sizeof(out),
		};

		ret = devm_scmi_process_msg(dev, &msg);
		if (ret)
			return ret;

		strncpy(name, out.clock_name, SCMI_CLOCK_NAME_LENGTH_MAX);
		*attr = out.attributes;
	} else {
		struct scmi_clk_attribute_out out;
		struct scmi_msg msg = {
			.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
			.message_id = SCMI_CLOCK_ATTRIBUTES,
			.in_msg = (u8 *)&in,
			.in_msg_sz = sizeof(in),
			.out_msg = (u8 *)&out,
			.out_msg_sz = sizeof(out),
		};

		ret = devm_scmi_process_msg(dev, &msg);
		if (ret)
			return ret;

		strncpy(name, out.clock_name, SCMI_CLOCK_NAME_LENGTH_MAX);
		*attr = out.attributes;
	}

	return 0;
}

static int scmi_clk_gate(struct clk *clk, int enable)
{
	struct scmi_clock_priv *priv;
	struct scmi_clk_state_in_v1 in_v1 = {
		.clock_id = clk_get_id(clk),
		.attributes = enable,
	};
	/* Valid only from SCMI clock v2.1 */
	struct scmi_clk_state_in_v2 in_v2 = {
		.clock_id = clk_get_id(clk),
		.attributes = enable,
	};
	struct scmi_clk_state_out out;
	struct scmi_msg msg_v1 = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					     SCMI_CLOCK_CONFIG_SET,
					     in_v1, out);
	struct scmi_msg msg_v2 = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					     SCMI_CLOCK_CONFIG_SET,
					     in_v2, out);
	int ret;

	/*
	 * In scmi_clk_probe(), in case of CLK_CCF is set, SCMI clock
	 * version is set in dev's parent priv struct. Otherwise
	 * SCMI clock version is set in dev priv struct.
	 */
	if (CONFIG_IS_ENABLED(CLK_CCF))
		priv = dev_get_parent_priv(clk->dev);
	else
		priv = dev_get_priv(clk->dev);

	ret = devm_scmi_process_msg(clk->dev,
				    (priv->version < CLOCK_PROTOCOL_VERSION_2_1) ?
				    &msg_v1 : &msg_v2);
	if (ret)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_clk_get_ctrl_flags(struct clk *clk, u32 *ctrl_flags)
{
	struct clk_scmi *clkscmi;
	struct udevice *dev;
	u32 attributes;
	struct clk *c;
	int ret;

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	dev = c->dev->parent;

	clkscmi = container_of(c, struct clk_scmi, clk);

	if (!clkscmi->attrs_resolved) {
		char name[SCMI_CLOCK_NAME_LENGTH_MAX];
		ret = scmi_clk_get_attribute(dev, clk->id & CLK_ID_MSK,
					     name, &attributes);
		if (ret)
			return ret;

		strncpy(clkscmi->name, name, SCMI_CLOCK_NAME_LENGTH_MAX);
		if (CLK_HAS_RESTRICTIONS(attributes)) {
			u32 perm;

			ret = scmi_clk_get_permissions(dev, clk->id & CLK_ID_MSK, &perm);
			if (ret < 0)
				clkscmi->ctrl_flags = 0;
			else
				clkscmi->ctrl_flags = perm;
		} else {
			clkscmi->ctrl_flags = SUPPORT_CLK_STAT_CONTROL |
					      SUPPORT_CLK_PARENT_CONTROL |
					      SUPPORT_CLK_RATE_CONTROL;
		}

		clkscmi->attrs_resolved = true;
	}

	*ctrl_flags = clkscmi->ctrl_flags;

	return 0;
}

static int scmi_clk_enable(struct clk *clk)
{
	u32 ctrl_flags;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return scmi_clk_gate(clk, 1);

	ret = scmi_clk_get_ctrl_flags(clk, &ctrl_flags);
	if (ret)
		return ret;

	if (ctrl_flags & SUPPORT_CLK_STAT_CONTROL)
		return scmi_clk_gate(clk, 1);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	dev_dbg(clk->dev, "%s: SCMI CLOCK: the clock cannot be enabled by the agent.\n", __func__);
	return 0;
}

static int scmi_clk_disable(struct clk *clk)
{
	u32 ctrl_flags;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return scmi_clk_gate(clk, 0);

	ret = scmi_clk_get_ctrl_flags(clk, &ctrl_flags);
	if (ret)
		return ret;

	if (ctrl_flags & SUPPORT_CLK_STAT_CONTROL)
		return scmi_clk_gate(clk, 0);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	dev_dbg(clk->dev,
		"%s: SCMI CLOCK: the clock cannot be disabled by the agent.\n", __func__);
	return 0;
}

static ulong scmi_clk_get_rate(struct clk *clk)
{
	struct scmi_clk_rate_get_in in = {
		.clock_id = clk_get_id(clk),
	};
	struct scmi_clk_rate_get_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					  SCMI_CLOCK_RATE_GET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(clk->dev, &msg);
	if (ret < 0)
		return ret;

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0)
		return ret;

	return (ulong)(((u64)out.rate_msb << 32) | out.rate_lsb);
}

static ulong __scmi_clk_set_rate(struct clk *clk, ulong rate)
{
	struct scmi_clk_rate_set_in in = {
		.clock_id = clk_get_id(clk),
		.flags = SCMI_CLK_RATE_ROUND_CLOSEST,
		.rate_lsb = (u32)rate,
		.rate_msb = (u32)((u64)rate >> 32),
	};
	struct scmi_clk_rate_set_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					  SCMI_CLOCK_RATE_SET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(clk->dev, &msg);
	if (ret < 0)
		return ret;

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0)
		return ret;

	return scmi_clk_get_rate(clk);
}

static ulong scmi_clk_set_rate(struct clk *clk, ulong rate)
{
	u32 ctrl_flags;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return __scmi_clk_set_rate(clk, rate);

	ret = scmi_clk_get_ctrl_flags(clk, &ctrl_flags);
	if (ret)
		return ret;

	if (ctrl_flags & SUPPORT_CLK_RATE_CONTROL)
		return __scmi_clk_set_rate(clk, rate);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	dev_dbg(clk->dev,
		"%s: SCMI CLOCK: the clock rate cannot be changed by the agent.\n", __func__);
	return 0;
}

static int scmi_clk_probe(struct udevice *dev)
{
	struct clk_scmi *clk_scmi_bulk, *clk_scmi;
	struct scmi_clock_priv *priv = dev_get_priv(dev);
	size_t num_clocks, i;
	int ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret)
		return ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return 0;

	/* register CCF children: CLK UCLASS, no probed again */
	if (device_get_uclass_id(dev->parent) == UCLASS_CLK)
		return 0;

	ret = scmi_clk_get_num_clock(dev, &num_clocks);
	if (ret)
		return ret;

	ret = scmi_generic_protocol_version(dev, SCMI_PROTOCOL_ID_CLOCK, &priv->version);
	if (ret) {
		dev_dbg(dev, "%s: get SCMI clock management protocol version failed\n", __func__);
		return ret;
	}

	clk_scmi_bulk = kzalloc(num_clocks * sizeof(*clk_scmi), GFP_KERNEL);
	if (!clk_scmi_bulk)
		return -ENOMEM;

	for (i = 0; i < num_clocks; i++) {
		clk_scmi = clk_scmi_bulk + i;
		char *clock_name = clk_scmi->name;

		snprintf(clock_name, SCMI_CLOCK_NAME_LENGTH_MAX, "scmi-%zu", i);

		ret = clk_register(&clk_scmi->clk, dev->driver->name,
				   clock_name, dev->name);
		if (ret)
			return ret;

		dev_clk_dm(dev, i, &clk_scmi->clk);
		dev_set_parent_priv(clk_scmi->clk.dev, priv);
	}

	return 0;
}

static int __scmi_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct scmi_clk_parent_set_in in = {
		.clock_id = clk_get_id(clk),
		.parent_clk = clk_get_id(parent),
	};
	struct scmi_clk_parent_set_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					  SCMI_CLOCK_PARENT_SET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(clk->dev, &msg);
	if (ret < 0)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_clk_set_parent(struct clk *clk, struct clk *parent)
{
	u32 ctrl_flags;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return __scmi_clk_set_parent(clk, parent);

	ret = scmi_clk_get_ctrl_flags(clk, &ctrl_flags);
	if (ret)
		return ret;

	if (ctrl_flags & SUPPORT_CLK_PARENT_CONTROL)
		return __scmi_clk_set_parent(clk, parent);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	dev_dbg(clk->dev,
		"%s: SCMI CLOCK: the clock's parent cannot be changed by the agent.\n", __func__);
	return 0;
}

static const struct clk_ops scmi_clk_ops = {
	.enable = scmi_clk_enable,
	.disable = scmi_clk_disable,
	.get_rate = scmi_clk_get_rate,
	.set_rate = scmi_clk_set_rate,
	.set_parent = scmi_clk_set_parent,
};

U_BOOT_DRIVER(scmi_clock) = {
	.name = "scmi_clk",
	.id = UCLASS_CLK,
	.ops = &scmi_clk_ops,
	.probe = scmi_clk_probe,
	.priv_auto = sizeof(struct scmi_clock_priv),
};

static struct scmi_proto_match match[] = {
	{ .proto_id = SCMI_PROTOCOL_ID_CLOCK },
	{ /* Sentinel */ }
};

U_BOOT_SCMI_PROTO_DRIVER(scmi_clock, match);
