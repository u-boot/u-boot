// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>
#include <asm/io.h>
#include <asm/scmi_test.h>
#include <dm/device_compat.h>

/*
 * The sandbox SCMI agent driver simulates to some extend a SCMI message
 * processing. It simulates few of the SCMI services for some of the
 * SCMI protocols embedded in U-Boot. Currently:
 * - SCMI clock protocol emulates an agent exposing 2 clocks
 * - SCMI reset protocol emulates an agent exposing a reset controller
 * - SCMI voltage domain protocol emulates an agent exposing 2 regulators
 *
 * As per DT bindings, the device node name shall be scmi.
 *
 * All clocks and regulators are default disabled and reset controller down.
 *
 * This driver exports sandbox_scmi_service_ctx() for the test sequence to
 * get the state of the simulated services (clock state, rate, ...) and
 * check back-end device state reflects the request send through the
 * various uclass devices, as clocks and reset controllers.
 */

static struct sandbox_scmi_clk scmi_clk[] = {
	{ .rate = 333 },
	{ .rate = 200 },
	{ .rate = 1000 },
};

static struct sandbox_scmi_reset scmi_reset[] = {
	{ .id = 3 },
};

static struct sandbox_scmi_voltd scmi_voltd[] = {
	{ .id = 0, .voltage_uv = 3300000 },
	{ .id = 1, .voltage_uv = 1800000 },
};

static struct sandbox_scmi_service sandbox_scmi_service_state;

struct sandbox_scmi_service *sandbox_scmi_service_ctx(void)
{
	return &sandbox_scmi_service_state;
}

static void debug_print_agent_state(struct udevice *dev, char *str)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);

	dev_dbg(dev, "Dump sandbox_scmi_agent: %s\n", str);
	dev_dbg(dev, " scmi_clk   (%zu): %d/%ld, %d/%ld, %d/%ld, ...\n",
		agent->clk_count,
		agent->clk_count ? agent->clk[0].enabled : -1,
		agent->clk_count ? agent->clk[0].rate : -1,
		agent->clk_count > 1 ? agent->clk[1].enabled : -1,
		agent->clk_count > 1 ? agent->clk[1].rate : -1,
		agent->clk_count > 2 ? agent->clk[2].enabled : -1,
		agent->clk_count > 2 ? agent->clk[2].rate : -1);
	dev_dbg(dev, " scmi_reset (%zu): %d, %d, ...\n",
		agent->reset_count,
		agent->reset_count ? agent->reset[0].asserted : -1,
		agent->reset_count > 1 ? agent->reset[1].asserted : -1);
	dev_dbg(dev, " scmi_voltd (%zu): %u/%d, %u/%d, ...\n",
		agent->voltd_count,
		agent->voltd_count ? agent->voltd[0].enabled : -1,
		agent->voltd_count ? agent->voltd[0].voltage_uv : -1,
		agent->voltd_count ? agent->voltd[1].enabled : -1,
		agent->voltd_count ? agent->voltd[1].voltage_uv : -1);
};

static struct sandbox_scmi_clk *get_scmi_clk_state(uint clock_id)
{
	if (clock_id < ARRAY_SIZE(scmi_clk))
		return scmi_clk + clock_id;

	return NULL;
}

static struct sandbox_scmi_reset *get_scmi_reset_state(uint reset_id)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(scmi_reset); n++)
		if (scmi_reset[n].id == reset_id)
			return scmi_reset + n;

	return NULL;
}

static struct sandbox_scmi_voltd *get_scmi_voltd_state(uint domain_id)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(scmi_voltd); n++)
		if (scmi_voltd[n].id == domain_id)
			return scmi_voltd + n;

	return NULL;
}

/*
 * Sandbox SCMI agent ops
 */

static int sandbox_scmi_clock_protocol_attribs(struct udevice *dev,
					       struct scmi_msg *msg)
{
	struct scmi_clk_protocol_attr_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_clk_protocol_attr_out *)msg->out_msg;
	out->attributes = ARRAY_SIZE(scmi_clk);
	out->status = SCMI_SUCCESS;

	return 0;
}

static int sandbox_scmi_clock_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_clk_attribute_in *in = NULL;
	struct scmi_clk_attribute_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;
	int ret;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_attribute_in *)msg->in_msg;
	out = (struct scmi_clk_attribute_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		memset(out, 0, sizeof(*out));

		if (clk_state->enabled)
			out->attributes = 1;

		ret = snprintf(out->clock_name, sizeof(out->clock_name),
			       "clk%u", in->clock_id);
		assert(ret > 0 && ret < sizeof(out->clock_name));

		out->status = SCMI_SUCCESS;
	}

	return 0;
}
static int sandbox_scmi_clock_rate_set(struct udevice *dev,
				       struct scmi_msg *msg)
{
	struct scmi_clk_rate_set_in *in = NULL;
	struct scmi_clk_rate_set_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_rate_set_in *)msg->in_msg;
	out = (struct scmi_clk_rate_set_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		u64 rate = ((u64)in->rate_msb << 32) + in->rate_lsb;

		clk_state->rate = (ulong)rate;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_clock_rate_get(struct udevice *dev,
				       struct scmi_msg *msg)
{
	struct scmi_clk_rate_get_in *in = NULL;
	struct scmi_clk_rate_get_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_rate_get_in *)msg->in_msg;
	out = (struct scmi_clk_rate_get_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		out->rate_msb = (u32)((u64)clk_state->rate >> 32);
		out->rate_lsb = (u32)clk_state->rate;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_clock_gate(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_clk_state_in *in = NULL;
	struct scmi_clk_state_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_state_in *)msg->in_msg;
	out = (struct scmi_clk_state_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else if (in->attributes > 1) {
		out->status = SCMI_PROTOCOL_ERROR;
	} else {
		clk_state->enabled = in->attributes;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_rd_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_rd_attr_in *in = NULL;
	struct scmi_rd_attr_out *out = NULL;
	struct sandbox_scmi_reset *reset_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_rd_attr_in *)msg->in_msg;
	out = (struct scmi_rd_attr_out *)msg->out_msg;

	reset_state = get_scmi_reset_state(in->domain_id);
	if (!reset_state) {
		dev_err(dev, "Unexpected reset domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		memset(out, 0, sizeof(*out));
		snprintf(out->name, sizeof(out->name), "rd%u", in->domain_id);

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_rd_reset(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_rd_reset_in *in = NULL;
	struct scmi_rd_reset_out *out = NULL;
	struct sandbox_scmi_reset *reset_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_rd_reset_in *)msg->in_msg;
	out = (struct scmi_rd_reset_out *)msg->out_msg;

	reset_state = get_scmi_reset_state(in->domain_id);
	if (!reset_state) {
		dev_err(dev, "Unexpected reset domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else if (in->reset_state > 1) {
		dev_err(dev, "Invalid reset domain input attribute value\n");

		out->status = SCMI_INVALID_PARAMETERS;
	} else {
		if (in->flags & SCMI_RD_RESET_FLAG_CYCLE) {
			if (in->flags & SCMI_RD_RESET_FLAG_ASYNC) {
				out->status = SCMI_NOT_SUPPORTED;
			} else {
				/* Ends deasserted whatever current state */
				reset_state->asserted = false;
				out->status = SCMI_SUCCESS;
			}
		} else {
			reset_state->asserted = in->flags &
						SCMI_RD_RESET_FLAG_ASSERT;

			out->status = SCMI_SUCCESS;
		}
	}

	return 0;
}

static int sandbox_scmi_voltd_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_voltd_attr_in *in = NULL;
	struct scmi_voltd_attr_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_attr_in *)msg->in_msg;
	out = (struct scmi_voltd_attr_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		memset(out, 0, sizeof(*out));
		snprintf(out->name, sizeof(out->name), "regu%u", in->domain_id);

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_config_set(struct udevice *dev,
					 struct scmi_msg *msg)
{
	struct scmi_voltd_config_set_in *in = NULL;
	struct scmi_voltd_config_set_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_config_set_in *)msg->in_msg;
	out = (struct scmi_voltd_config_set_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else if (in->config & ~SCMI_VOLTD_CONFIG_MASK) {
		dev_err(dev, "Invalid config value 0x%x\n", in->config);

		out->status = SCMI_INVALID_PARAMETERS;
	} else if (in->config != SCMI_VOLTD_CONFIG_ON &&
		   in->config != SCMI_VOLTD_CONFIG_OFF) {
		dev_err(dev, "Unexpected custom value 0x%x\n", in->config);

		out->status = SCMI_INVALID_PARAMETERS;
	} else {
		voltd_state->enabled = in->config == SCMI_VOLTD_CONFIG_ON;
		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_config_get(struct udevice *dev,
					 struct scmi_msg *msg)
{
	struct scmi_voltd_config_get_in *in = NULL;
	struct scmi_voltd_config_get_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_config_get_in *)msg->in_msg;
	out = (struct scmi_voltd_config_get_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		if (voltd_state->enabled)
			out->config = SCMI_VOLTD_CONFIG_ON;
		else
			out->config = SCMI_VOLTD_CONFIG_OFF;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_level_set(struct udevice *dev,
					 struct scmi_msg *msg)
{
	struct scmi_voltd_level_set_in *in = NULL;
	struct scmi_voltd_level_set_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_level_set_in *)msg->in_msg;
	out = (struct scmi_voltd_level_set_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		voltd_state->voltage_uv = in->voltage_level;
		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_level_get(struct udevice *dev,
					struct scmi_msg *msg)
{
	struct scmi_voltd_level_get_in *in = NULL;
	struct scmi_voltd_level_get_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_level_get_in *)msg->in_msg;
	out = (struct scmi_voltd_level_get_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		out->voltage_level = voltd_state->voltage_uv;
		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_test_process_msg(struct udevice *dev,
					 struct scmi_msg *msg)
{
	switch (msg->protocol_id) {
	case SCMI_PROTOCOL_ID_CLOCK:
		switch (msg->message_id) {
		case SCMI_PROTOCOL_ATTRIBUTES:
			return sandbox_scmi_clock_protocol_attribs(dev, msg);
		case SCMI_CLOCK_ATTRIBUTES:
			return sandbox_scmi_clock_attribs(dev, msg);
		case SCMI_CLOCK_RATE_SET:
			return sandbox_scmi_clock_rate_set(dev, msg);
		case SCMI_CLOCK_RATE_GET:
			return sandbox_scmi_clock_rate_get(dev, msg);
		case SCMI_CLOCK_CONFIG_SET:
			return sandbox_scmi_clock_gate(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_RESET_DOMAIN:
		switch (msg->message_id) {
		case SCMI_RESET_DOMAIN_ATTRIBUTES:
			return sandbox_scmi_rd_attribs(dev, msg);
		case SCMI_RESET_DOMAIN_RESET:
			return sandbox_scmi_rd_reset(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN:
		switch (msg->message_id) {
		case SCMI_VOLTAGE_DOMAIN_ATTRIBUTES:
			return sandbox_scmi_voltd_attribs(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_CONFIG_SET:
			return sandbox_scmi_voltd_config_set(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_CONFIG_GET:
			return sandbox_scmi_voltd_config_get(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_LEVEL_SET:
			return sandbox_scmi_voltd_level_set(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_LEVEL_GET:
			return sandbox_scmi_voltd_level_get(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_BASE:
	case SCMI_PROTOCOL_ID_POWER_DOMAIN:
	case SCMI_PROTOCOL_ID_SYSTEM:
	case SCMI_PROTOCOL_ID_PERF:
	case SCMI_PROTOCOL_ID_SENSOR:
		*(u32 *)msg->out_msg = SCMI_NOT_SUPPORTED;
		return 0;
	default:
		break;
	}

	dev_err(dev, "%s(%s): Unhandled protocol_id %#x/message_id %#x\n",
		__func__, dev->name, msg->protocol_id, msg->message_id);

	if (msg->out_msg_sz < sizeof(u32))
		return -EINVAL;

	/* Intentionnaly report unhandled IDs through the SCMI return code */
	*(u32 *)msg->out_msg = SCMI_PROTOCOL_ERROR;
	return 0;
}

static int sandbox_scmi_test_remove(struct udevice *dev)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);

	if (agent != sandbox_scmi_service_ctx()->agent)
		return -EINVAL;

	debug_print_agent_state(dev, "removed");

	/* We only need to dereference the agent in the context */
	sandbox_scmi_service_ctx()->agent = NULL;

	return 0;
}

static int sandbox_scmi_test_probe(struct udevice *dev)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);

	if (sandbox_scmi_service_ctx()->agent)
		return -EINVAL;

	*agent = (struct sandbox_scmi_agent){
		.clk = scmi_clk,
		.clk_count = ARRAY_SIZE(scmi_clk),
		.reset = scmi_reset,
		.reset_count = ARRAY_SIZE(scmi_reset),
		.voltd = scmi_voltd,
		.voltd_count = ARRAY_SIZE(scmi_voltd),
	};

	debug_print_agent_state(dev, "probed");

	/* Save reference for tests purpose */
	sandbox_scmi_service_ctx()->agent = agent;

	return 0;
};

static const struct udevice_id sandbox_scmi_test_ids[] = {
	{ .compatible = "sandbox,scmi-agent" },
	{ }
};

struct scmi_agent_ops sandbox_scmi_test_ops = {
	.process_msg = sandbox_scmi_test_process_msg,
};

U_BOOT_DRIVER(sandbox_scmi_agent) = {
	.name = "sandbox-scmi_agent",
	.id = UCLASS_SCMI_AGENT,
	.of_match = sandbox_scmi_test_ids,
	.priv_auto	= sizeof(struct sandbox_scmi_agent),
	.probe = sandbox_scmi_test_probe,
	.remove = sandbox_scmi_test_remove,
	.ops = &sandbox_scmi_test_ops,
};
