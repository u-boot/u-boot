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
 * - SCMI clock protocol: emulate 2 agents each exposing few clocks
 * - SCMI reset protocol: emulate 1 agents each exposing a reset
 *
 * Agent #0 simulates 2 clocks and 1 reset domain.
 * See IDs in scmi0_clk[]/scmi0_reset[] and "sandbox-scmi-agent@0" in test.dts.
 *
 * Agent #1 simulates 1 clock.
 * See IDs in scmi1_clk[] and "sandbox-scmi-agent@1" in test.dts.
 *
 * All clocks are default disabled and reset levels down.
 *
 * This Driver exports sandbox_scmi_service_ct() for the test sequence to
 * get the state of the simulated services (clock state, rate, ...) and
 * check back-end device state reflects the request send through the
 * various uclass devices, as clocks and reset controllers.
 */

#define SANDBOX_SCMI_AGENT_COUNT	2

static struct sandbox_scmi_clk scmi0_clk[] = {
	{ .id = 7, .rate = 1000 },
	{ .id = 3, .rate = 333 },
};

static struct sandbox_scmi_reset scmi0_reset[] = {
	{ .id = 3 },
};

static struct sandbox_scmi_clk scmi1_clk[] = {
	{ .id = 1, .rate = 44 },
};

/* The list saves to simulted end devices references for test purpose */
struct sandbox_scmi_agent *sandbox_scmi_agent_list[SANDBOX_SCMI_AGENT_COUNT];

static struct sandbox_scmi_service sandbox_scmi_service_state = {
	.agent = sandbox_scmi_agent_list,
	.agent_count = SANDBOX_SCMI_AGENT_COUNT,
};

struct sandbox_scmi_service *sandbox_scmi_service_ctx(void)
{
	return &sandbox_scmi_service_state;
}

static void debug_print_agent_state(struct udevice *dev, char *str)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);

	dev_dbg(dev, "Dump sandbox_scmi_agent %u: %s\n", agent->idx, str);
	dev_dbg(dev, " scmi%u_clk   (%zu): %d/%ld, %d/%ld, %d/%ld, ...\n",
		agent->idx,
		agent->clk_count,
		agent->clk_count ? agent->clk[0].enabled : -1,
		agent->clk_count ? agent->clk[0].rate : -1,
		agent->clk_count > 1 ? agent->clk[1].enabled : -1,
		agent->clk_count > 1 ? agent->clk[1].rate : -1,
		agent->clk_count > 2 ? agent->clk[2].enabled : -1,
		agent->clk_count > 2 ? agent->clk[2].rate : -1);
	dev_dbg(dev, " scmi%u_reset (%zu): %d, %d, ...\n",
		agent->idx,
		agent->reset_count,
		agent->reset_count ? agent->reset[0].asserted : -1,
		agent->reset_count > 1 ? agent->reset[1].asserted : -1);
};

static struct sandbox_scmi_clk *get_scmi_clk_state(uint agent_id, uint clock_id)
{
	struct sandbox_scmi_clk *target = NULL;
	size_t target_count = 0;
	size_t n;

	switch (agent_id) {
	case 0:
		target = scmi0_clk;
		target_count = ARRAY_SIZE(scmi0_clk);
		break;
	case 1:
		target = scmi1_clk;
		target_count = ARRAY_SIZE(scmi1_clk);
		break;
	default:
		return NULL;
	}

	for (n = 0; n < target_count; n++)
		if (target[n].id == clock_id)
			return target + n;

	return NULL;
}

static struct sandbox_scmi_reset *get_scmi_reset_state(uint agent_id,
						       uint reset_id)
{
	size_t n;

	if (agent_id == 0) {
		for (n = 0; n < ARRAY_SIZE(scmi0_reset); n++)
			if (scmi0_reset[n].id == reset_id)
				return scmi0_reset + n;
	}

	return NULL;
}

/*
 * Sandbox SCMI agent ops
 */

static int sandbox_scmi_clock_rate_set(struct udevice *dev,
				       struct scmi_msg *msg)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);
	struct scmi_clk_rate_set_in *in = NULL;
	struct scmi_clk_rate_set_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_rate_set_in *)msg->in_msg;
	out = (struct scmi_clk_rate_set_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(agent->idx, in->clock_id);
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
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);
	struct scmi_clk_rate_get_in *in = NULL;
	struct scmi_clk_rate_get_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_rate_get_in *)msg->in_msg;
	out = (struct scmi_clk_rate_get_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(agent->idx, in->clock_id);
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
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);
	struct scmi_clk_state_in *in = NULL;
	struct scmi_clk_state_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_state_in *)msg->in_msg;
	out = (struct scmi_clk_state_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(agent->idx, in->clock_id);
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
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);
	struct scmi_rd_attr_in *in = NULL;
	struct scmi_rd_attr_out *out = NULL;
	struct sandbox_scmi_reset *reset_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_rd_attr_in *)msg->in_msg;
	out = (struct scmi_rd_attr_out *)msg->out_msg;

	reset_state = get_scmi_reset_state(agent->idx, in->domain_id);
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
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);
	struct scmi_rd_reset_in *in = NULL;
	struct scmi_rd_reset_out *out = NULL;
	struct sandbox_scmi_reset *reset_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_rd_reset_in *)msg->in_msg;
	out = (struct scmi_rd_reset_out *)msg->out_msg;

	reset_state = get_scmi_reset_state(agent->idx, in->domain_id);
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

static int sandbox_scmi_test_process_msg(struct udevice *dev,
					 struct scmi_msg *msg)
{
	switch (msg->protocol_id) {
	case SCMI_PROTOCOL_ID_CLOCK:
		switch (msg->message_id) {
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

	debug_print_agent_state(dev, "removed");

	/* We only need to dereference the agent in the context */
	sandbox_scmi_service_ctx()->agent[agent->idx] = NULL;

	return 0;
}

static int sandbox_scmi_test_probe(struct udevice *dev)
{
	static const char basename[] = "sandbox-scmi-agent@";
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);
	const size_t basename_size = sizeof(basename) - 1;

	if (strncmp(basename, dev->name, basename_size))
		return -ENOENT;

	switch (dev->name[basename_size]) {
	case '0':
		*agent = (struct sandbox_scmi_agent){
			.idx = 0,
			.clk = scmi0_clk,
			.clk_count = ARRAY_SIZE(scmi0_clk),
			.reset = scmi0_reset,
			.reset_count = ARRAY_SIZE(scmi0_reset),
		};
		break;
	case '1':
		*agent = (struct sandbox_scmi_agent){
			.idx = 1,
			.clk = scmi1_clk,
			.clk_count = ARRAY_SIZE(scmi1_clk),
		};
		break;
	default:
		dev_err(dev, "%s(): Unexpected agent ID %s\n",
			__func__, dev->name + basename_size);
		return -ENOENT;
	}

	debug_print_agent_state(dev, "probed");

	/* Save reference for tests purpose */
	sandbox_scmi_service_ctx()->agent[agent->idx] = agent;

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
